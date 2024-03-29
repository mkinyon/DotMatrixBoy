
#include "WaveChannel.h"

namespace Core
{
	WaveChannel::WaveChannel(Mmu& mmu) : m_MMU(mmu) {}
	WaveChannel::~WaveChannel() {}

	void WaveChannel::Start()
	{
		if (!m_IsActive)
		{
			m_IsActive = true;
			Trigger();
		}
	}

	void WaveChannel::Stop()
	{
		m_IsActive = false;
		m_IsRunning = false;
	}

	void WaveChannel::Trigger()
	{
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

		uint8_t levelReg = m_MMU.Read(HW_FF1C_NR32_SOUND_CH3_OUTPUT_LEVEL);
		outputLevel = (Core::AUDIO_LEVEL)((levelReg & 0x60) >> 5);

		UpdateSample();
	}

	void WaveChannel::LengthClock()
	{
		m_IsActive = m_LengthComp.Clock();
		if (!m_IsActive)
		{
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
		// Each byte in the Wave RAM holds two 2 4-bit samples
		uint8_t sampleByte = m_MMU.Read(HW_FF30_WAVRAM_WAVEFORM_STORAGE_0 + (m_SampleIndex / 2));
		uint8_t newSample = m_SampleIndex % 2 == 0
			? (sampleByte & 0xF0) >> 4
			: sampleByte & 0x0F;

		m_CurrentSample = newSample;
		if (outputLevel != Core::AUDIO_LEVEL::MUTE)
		{
			m_CurrentSample = m_CurrentSample >> (static_cast<uint8_t>(outputLevel) - 1);
		}
			
		if (!m_IsActive || !m_IsRunning || outputLevel == Core::AUDIO_LEVEL::MUTE)
		{
			m_CurrentSample = 0;
		}
	}
}