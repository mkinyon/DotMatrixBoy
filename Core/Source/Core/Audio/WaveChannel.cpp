
#include "WaveChannel.h"

namespace Core
{
	WaveChannel::WaveChannel(Mmu& mmu) : m_MMU(mmu) {}
	WaveChannel::~WaveChannel() {}

	void WaveChannel::Start()
	{
		// DAC enable only enables the DAC, it doesn't retrigger the channel
		// The channel is only triggered by the trigger bit in NR34
		// Just ensure the channel can output if it's already running
		// Don't retrigger if already active or if length timer expired
	}

	void WaveChannel::Stop()
	{
		// DAC disable stops output immediately
		// Don't set m_IsRunning = false here, as that would prevent
		// the channel from resuming if DAC is re-enabled
		// The DAC state is checked in UpdateSample()
	}

	void WaveChannel::Trigger()
	{
		// Check if DAC is enabled - channel won't work if DAC is disabled
		if (!m_MMU.ReadRegisterBit(HW_FF1A_NR30_SOUND_CH3_DAC_ENABLE, NR30_DAC_ON_OFF))
		{
			return;
		}

		m_IsActive = true;
		m_IsRunning = true;
		m_MMU.WriteRegisterBit(HW_FF26_NR52_SOUND_TOGGLE, NR52_CH3_ON, true);

		uint16_t length = AUDIO_WAVE_LENGTH - m_MMU.Read(HW_FF1B_NR31_SOUND_CH3_LEN_TIMER);
		bool lengthStop = m_MMU.ReadRegisterBit(HW_FF1E_NR34_SOUND_CH3_PERIOD_HIGH, NR34_LEN_ENABLE);
		m_LengthComp.SetLength(length, lengthStop);

		// Nothing magical just the way the hardware calculates the channel frequency
		// see: https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware
		m_CycleSampleUpdate = (2048 - GetFrequency()) * 2;
		m_CycleCount = 0;
		m_SampleIndex = 0;

		UpdateSample();
	}

	void WaveChannel::LengthClock()
	{
		m_IsActive = m_LengthComp.Clock();
		if (!m_IsActive)
		{
			m_IsRunning = false;
			m_MMU.WriteRegisterBit(HW_FF26_NR52_SOUND_TOGGLE, NR52_CH3_ON, false);
		}
	}

	void WaveChannel::Clock()
	{
		m_CycleCount++;
		if (m_CycleCount >= m_CycleSampleUpdate)
		{
			m_SampleIndex++;
			if (m_SampleIndex > 31)
			{
				m_SampleIndex = 0;
			}

			UpdateSample();
			m_CycleCount -= m_CycleSampleUpdate;
		}
	}

	uint8_t WaveChannel::GetCurrentSample() const
	{
		return m_CurrentSample;
	}

	uint16_t WaveChannel::GetFrequency() const
	{
		uint8_t frequencyData = m_MMU.Read(HW_FF1E_NR34_SOUND_CH3_PERIOD_HIGH);
		uint16_t frequency = m_MMU.Read(HW_FF1D_NR33_SOUND_CH3_PERIOD_LOW);
		frequency |= (frequencyData & 0x7) << 8;
		return frequency;
	}

	void WaveChannel::UpdateSample()
	{
		// Check if DAC is enabled - if not, output 0
		if (!m_MMU.ReadRegisterBit(HW_FF1A_NR30_SOUND_CH3_DAC_ENABLE, NR30_DAC_ON_OFF))
		{
			m_CurrentSample = 0;
			return;
		}

		// Check if channel should output anything
		if (!m_IsActive || !m_IsRunning)
		{
			m_CurrentSample = 0;
			return;
		}

		// Read output level from register (can change at any time)
		uint8_t levelReg = m_MMU.Read(HW_FF1C_NR32_SOUND_CH3_OUTPUT_LEVEL);
		Core::AUDIO_LEVEL currentOutputLevel = (Core::AUDIO_LEVEL)((levelReg & 0x60) >> 5);

		// Check if muted
		if (currentOutputLevel == Core::AUDIO_LEVEL::MUTE)
		{
			m_CurrentSample = 0;
			return;
		}

		// Each byte in the Wave RAM holds two 4-bit samples
		uint8_t sampleByte = m_MMU.Read(HW_FF30_WAVRAM_WAVEFORM_STORAGE_0 + (m_SampleIndex / 2));
		uint8_t newSample = m_SampleIndex % 2 == 0
			? (sampleByte & 0xF0) >> 4
			: sampleByte & 0x0F;

		// Apply output level shift: 1=100%, 2=50%, 3=25%
		// Shift right by (level - 1): 1->0, 2->1, 3->2
		m_CurrentSample = newSample >> (static_cast<uint8_t>(currentOutputLevel) - 1);
	}
}