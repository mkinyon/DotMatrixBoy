
#include "NoiseChannel.h"

namespace Core
{
	NoiseChannel::NoiseChannel(Mmu& mmu) : m_MMU(mmu) {}
	NoiseChannel::~NoiseChannel() {}

	void NoiseChannel::Trigger()
	{
		m_IsActive = true;
		m_MMU.WriteRegisterBit(HW_FF26_NR52_SOUND_TOGGLE, NR52_CH4_ON, true);

		// setup length
		uint16_t length = APU_DEFAULT_LENGTH - (m_MMU.Read(HW_FF20_NR41_SOUND_CH4_LEN_TIMER) & 0b111111);
		bool lengthStop = m_MMU.ReadRegisterBit(HW_FF23_NR44_SOUND_CH4_CONTROL, NR44_LEN_ENABLE);
		m_LengthComp.SetLength(length, lengthStop);

		// setup envelope
		m_EnvelopeComp.SetEnvelope(
			m_MMU.Read(HW_FF21_NR42_SOUND_CH4_VOL_ENVELOPE) & 0x7, // grab bits 0, 1, 2
			(m_MMU.Read(HW_FF21_NR42_SOUND_CH4_VOL_ENVELOPE) & 240) >> 4,
			m_MMU.Read(HW_FF21_NR42_SOUND_CH4_VOL_ENVELOPE) & 8
		);

		// noise
		uint8_t noiseData = m_MMU.Read(HW_FF22_NR43_SOUND_CH4_FREQ_RANDOM);
		m_Divisor = m_AudioDivisorArray[noiseData & 0x7];
		m_IsWidth7Bit = noiseData & 0x8;
		m_lfsr = 0x7FFF;

		// Frequency calculation: Effective Divisor = Base Divisor × 2^(s+1)
		uint8_t clockShift = (noiseData & 0xF0) >> 4;
		m_CycleSampleUpdate = m_Divisor << (clockShift + 1);
		m_CycleCount = 0;

		UpdateSample();
	}

	void NoiseChannel::Clock()
	{
		m_CycleCount++;
		if (m_CycleCount >= m_CycleSampleUpdate)
		{
			UpdateSample();
			m_CycleCount -= m_CycleSampleUpdate;
		}
	}

	void NoiseChannel::LengthClock()
	{
		m_IsActive = m_LengthComp.Clock();
		if (!m_IsActive)
		{
			m_MMU.WriteRegisterBit(HW_FF26_NR52_SOUND_TOGGLE, NR52_CH4_ON, false);
		}
	}

	void NoiseChannel::EnvelopeClock()
	{
		m_EnvelopeComp.Clock();
	}

	uint8_t NoiseChannel::GetCurrentSample()
	{
		return m_CurrentSample;
	}

	void NoiseChannel::UpdateSample()
	{
		uint8_t bit0 = m_lfsr & 0x1;
		uint8_t bit1 = (m_lfsr & 0x2) >> 1;
		uint8_t xored = bit0 ^ bit1;

		// Shift right by one bit
		m_lfsr >>= 1;
		
		// Insert XOR result into bit 14
		m_lfsr |= (xored << 14);
		
		// In 7-bit mode, also insert XOR result into bit 6
		if (m_IsWidth7Bit)
		{
			m_lfsr &= ~(1 << 6);  // Clear bit 6
			m_lfsr |= (xored << 6); // Set bit 6 to XOR result
		}

		m_CurrentSample = (m_lfsr & 0x1) == 0 ? 1 : 0;
		m_CurrentSample *= m_EnvelopeComp.GetVolume();

		if (!m_IsActive)
		{
			m_CurrentSample = 0;
		}
	}
}