
#include "SquareChannel.h"


namespace Core
{
	SquareChannel::SquareChannel(Mmu& mmu, bool isChannel1) : m_MMU(mmu)
	{
		this->m_IsChannel1 = isChannel1;
	}

	SquareChannel::~SquareChannel() {}

	void SquareChannel::Clock()
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

	void SquareChannel::LengthClock()
	{
		if (m_LengthCounter > 0 && m_LengthEnable) {
			m_LengthCounter--;
			if (m_LengthCounter == 0) {
				m_IsActive = false;	// Disable channel
			}
		}
	}

	void SquareChannel::SweepClock()
	{

	}

	void SquareChannel::EnvelopeClock()
	{
		
	}

	void SquareChannel::Trigger()
	{
		m_IsActive = true;
		m_MMU.WriteRegisterBit(HW_NR52_SOUND_TOGGLE, m_SoundControlFlag, true);

		m_SelectedDuty = (m_MMU.Read(m_LengthDutyAddr) & 0b11000000) >> 6;

		// https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Square_Wave
		// a square channel's frequency timer period is set to (2048-frequency)*4
		m_CurrentFrequency = GetFrequency();
		m_CycleSampleUpdate = (2048 - m_CurrentFrequency) * 4;
		m_CycleCount = 0;
		m_SampleIndex = 0;

		// setup length
		uint16_t initLength = APU_DEFAULT_LENGTH - (m_MMU.Read(m_LengthDutyAddr) & 0b111111);
		m_LengthCounter = m_LengthCounter == 0 ? initLength : m_LengthCounter;
		m_LengthEnable = m_MMU.ReadRegisterBit(m_DataAddr, NR14_LEN_ENABLE);
	}

	uint16_t SquareChannel::GetFrequency()
	{
		uint8_t frequencyData = m_MMU.Read(m_DataAddr);
		uint16_t frequency = m_MMU.Read(m_FreqLowAddr);
		frequency |= (frequencyData & 0b111) << 8;
		return frequency;
	}

	void SquareChannel::UpdateSample()
	{
		uint8_t dutyValue = m_DutyCycleTable[m_SelectedDuty][m_SampleIndex];
		m_CurrentSample = dutyValue;

		if (!m_IsActive)
		{
			m_CurrentSample = 0;
		}
	}
}