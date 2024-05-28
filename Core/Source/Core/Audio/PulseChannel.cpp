
#include "PulseChannel.h"


namespace Core
{
	PulseChannel::PulseChannel(Mmu& mmu, bool isChannel1) : m_MMU(mmu)
	{
		bool m_HasSweep = isChannel1;
		m_SoundControlFlag = isChannel1 ? NR52_CH1_ON : NR52_CH2_ON;
		m_DataAddr = isChannel1 ? HW_FF14_NR14_SOUND_CH1_PERIOD_HIGH : HW_FF19_NR24_SOUND_CH2_PERIOD_HIGH;
		m_FreqLowAddr = isChannel1 ? HW_FF13_NR13_SOUND_CH1_PERIOD_LOW : HW_FF18_NR23_SOUND_CH2_PERIOD_LOW;
		m_LengthDutyAddr = isChannel1 ? HW_FF11_NR11_SOUND_CH1_LEN_TIMER : HW_FF16_NR21_SOUND_CH2_LEN_TIMER;
		m_EvenlopeAddr = isChannel1 ? HW_FF12_NR12_SOUND_CH1_VOL_ENVELOPE : HW_FF17_NR22_SOUND_CH2_VOL_ENVELOPE;
		m_SweepAddr = isChannel1 ? HW_FF10_NR10_SOUND_CH1_SWEEP : 0;
	}

	PulseChannel::~PulseChannel() {}

	void PulseChannel::Clock()
	{
		m_CycleCount++;
		if (m_CycleCount >= m_CycleSampleUpdate)
		{
			m_SampleIndex++;
			if (m_SampleIndex > 7)
			{
				m_SampleIndex = 0;
			}

			UpdateSample();
			m_CycleCount -= m_CycleSampleUpdate;
		}
	}

	void PulseChannel::LengthClock()
	{
		m_IsActive = m_LengthComp.Clock();
		if (!m_IsActive)
		{
			m_MMU.WriteRegisterBit(HW_FF26_NR52_SOUND_TOGGLE, m_SoundControlFlag, false);
		}
	}

	void PulseChannel::SweepClock()
	{
		if (m_SweepTime == 0)
		{
			return;
		}

		if (m_ElaspsedSweepTime != m_SweepTime)
		{
			m_ElaspsedSweepTime++;
		}
			
		if (m_ElaspsedSweepTime == m_SweepTime)
		{
			int8_t sweepCorrection = m_IsSweepDecreasing ? -1 : 1;
			uint8_t sweepChange = (m_CurrentFrequency >> m_SweepShift) * sweepCorrection;

			// overflow on decrease - do nothing
			if (m_IsSweepDecreasing && sweepChange > m_CurrentFrequency)
			{
				m_ElaspsedSweepTime = 0;
			}
			// overflow on increase - stop channel
			else if (!m_IsSweepDecreasing && sweepChange + m_CurrentFrequency > 2047)
			{
				m_IsActive = false;
			}
			else
			{
				m_CurrentFrequency += sweepChange;
				m_CycleSampleUpdate = (2048 - m_CurrentFrequency) * 4;
				m_CycleCount = 0;
				m_ElaspsedSweepTime = 0;
				SetFrequency(m_CurrentFrequency);
			}
		}
	}

	void PulseChannel::EnvelopeClock()
	{
		m_EnvelopeComp.Clock();
	}

	void PulseChannel::Trigger()
	{
		m_IsActive = true;
		m_MMU.WriteRegisterBit(HW_FF26_NR52_SOUND_TOGGLE, m_SoundControlFlag, true);

		m_SelectedDuty = (m_MMU.Read(m_LengthDutyAddr) & 0b11000000) >> 6;
		
		// setup length
		uint16_t length = APU_DEFAULT_LENGTH - (m_MMU.Read(m_LengthDutyAddr) & 0b111111);
		bool lengthStop = m_MMU.ReadRegisterBit(m_DataAddr, NR14_LEN_ENABLE);
		m_LengthComp.SetLength(length, lengthStop);

		// setup envelope
		m_EnvelopeComp.SetEnvelope(
			m_MMU.Read(m_EvenlopeAddr) & 0x7, // grab bits 0, 1, 2
			(m_MMU.Read(m_EvenlopeAddr) & 240) >> 4,
			m_MMU.Read(m_EvenlopeAddr) & 8
		);

		// https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Square_Wave
		// a square channel's frequency timer period is set to (2048-frequency)*4
		m_CurrentFrequency = GetFrequency();
		m_CycleSampleUpdate = (2048 - m_CurrentFrequency) * 4;
		m_CycleCount = 0;
		m_SampleIndex = 0;

		// setup sweep
		if (m_HasSweep)
		{
			m_ElaspsedSweepTime = 0;

			uint8_t sweepData = m_MMU.Read(m_SweepAddr);
			m_SweepTime = (sweepData & 0x70) >> 0x4;
			m_IsSweepDecreasing = sweepData & 0x8;
			m_SweepShift = sweepData & 0x7;
		}
	}

	uint16_t PulseChannel::GetFrequency() const
	{
		// The frequency (period) is 11 bits is stored at two addresses. The
		// low 8 bits are stored in FF13 (ch1) and FF18 (ch2). The
		// upper 3 bits are stored in bits 0, 1, 2 at FF14 (ch1) and
		// FF19 (ch2).

		// the upper 3 bits
		uint8_t frequencyData = m_MMU.Read(m_DataAddr);

		// the lower 8 bits
		uint16_t frequency = m_MMU.Read(m_FreqLowAddr);

		// combine them into 11 bits and store in a 16 bit variable
		frequency |= (frequencyData & 0b111) << 8;

		return frequency;
	}

	void PulseChannel::SetFrequency(uint16_t frequency)
	{
		// read upper three bits from frequency (period)
		uint8_t frequencyData = m_MMU.Read(m_DataAddr);

		// AND 0xF8 deletes old frequency from the data and 0x700 takes the upper 3 bits of the frequency
		m_MMU.Write(m_DataAddr, (frequencyData & 0xF8) | ((frequency & 0x700) >> 8));

		// write lower 8 bits
		m_MMU.Write(m_FreqLowAddr, frequency & 0xFF);
	}

	void PulseChannel::UpdateSample()
	{
		m_CurrentSample = 0;

		if (m_IsActive)
		{
			uint8_t dutyValue = m_DutyCycleTable[m_SelectedDuty][m_SampleIndex];
			m_CurrentSample = dutyValue * m_EnvelopeComp.GetVolume();
		}
	}

	uint8_t PulseChannel::GetCurrentSample()
	{
		return m_CurrentSample;
	}
}