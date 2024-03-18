
#include "NoiseChannel.h"

namespace Core
{
	NoiseChannel::NoiseChannel(Mmu& mmu) : m_MMU(mmu) {}
	NoiseChannel::~NoiseChannel() {}

	void NoiseChannel::Trigger()
	{
		m_IsActive = true;
		m_MMU.WriteRegisterBit(HW_NR52_SOUND_TOGGLE, NR52_CH4_ON, true);

		// setup length
		uint16_t length = APU_DEFAULT_LENGTH - (m_MMU.Read(HW_NR42_SOUND_CHANNEL_4_VOL_ENVELOPE) & 0b111111);
		bool lengthStop = m_MMU.ReadRegisterBit(HW_NR41_SOUND_CHANNEL_4_LEN_TIMER, NR14_LEN_ENABLE);
		m_LengthComp.SetLength(length, lengthStop);

		// setup envelope
		m_EnvelopeComp.SetEnvelope(
			m_MMU.Read(HW_NR42_SOUND_CHANNEL_4_VOL_ENVELOPE) & 0x7, // grab bits 0, 1, 2
			(m_MMU.Read(HW_NR42_SOUND_CHANNEL_4_VOL_ENVELOPE) & 240) >> 4,
			m_MMU.Read(HW_NR42_SOUND_CHANNEL_4_VOL_ENVELOPE) & 8
		);

		// noise
		uint8_t noiseData = m_MMU.Read(HW_NR43_SOUND_CHANNEL_4_FREQ_RANDOM);
		m_Divisor = m_AudioDivisorArray[noiseData & 0x7];
		m_IsWidth7Bit = noiseData & 0x8;
		m_lfsr = 0x7FFF;

		m_CycleSampleUpdate = m_Divisor << ((noiseData & 0xF0) >> 4);
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
			m_MMU.WriteRegisterBit(HW_NR52_SOUND_TOGGLE, NR52_CH4_ON, false);
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

		m_lfsr = (xored << 14) | (m_lfsr >> 1);
		if (m_IsWidth7Bit)
		{
			m_lfsr = (xored << 6) | (m_lfsr & 0x7FBF);
		}

		m_CurrentSample = (m_lfsr & 0x1) == 0 ? 1 : 0;
		m_CurrentSample *= m_EnvelopeComp.GetVolume();

		if (!m_IsActive)
		{
			m_CurrentSample = 0;
		}
	}
}