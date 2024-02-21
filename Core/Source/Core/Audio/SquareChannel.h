#pragma once

#include "..\Mmu.h"
#include "..\Defines.h"

namespace Core
{
	class SquareChannel
	{
	public:
		SquareChannel(Mmu& mmu, bool isChannel1);
		~SquareChannel();

		void Clock();
		void LengthClock();
		void SweepClock();
		void EnvelopeClock();
		void Trigger();

	public:
		uint8_t m_CurrentSample = 0;

	private:
		void UpdateSample();
		uint16_t GetFrequency();

	private:
		Mmu& m_MMU;
		const bool m_DutyCycleTable[4][8] = {
		{0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 1, 1, 1},
		{0, 1, 1, 1, 1, 1, 1, 0}
		};

		bool m_IsActive = 0;
		uint8_t m_SelectedDuty = 0;
		uint16_t m_SampleIndex = 0;
		uint16_t m_CycleSampleUpdate = 0;
		uint16_t m_CurrentFrequency = 0;
		uint16_t m_CycleCount = 0;

		bool m_LengthEnable = false;
		uint8_t m_LengthLoad = 0;
		uint8_t m_LengthCounter = 0;

		// channel 1 & 2 parameters
		bool m_IsChannel1;
		bool m_HasSweep = m_IsChannel1;
		uint8_t  m_SoundControlFlag = m_IsChannel1 ?  NR52_CH1_ON: NR52_CH2_ON;
		uint16_t m_DataAddr = m_IsChannel1 ? HW_NR14_SOUND_CHANNEL_1_PERIOD_HIGH : HW_NR24_SOUND_CHANNEL_2_PERIOD_HIGH;
		uint16_t m_FreqLowAddr = m_IsChannel1 ? HW_NR13_SOUND_CHANNEL_1_PERIOD_LOW : HW_NR13_SOUND_CHANNEL_1_PERIOD_LOW;
		uint16_t m_LengthDutyAddr = m_IsChannel1 ? HW_NR11_SOUND_CHANNEL_1_LEN_TIMER : HW_NR21_SOUND_CHANNEL_2_LEN_TIMER;
		uint16_t m_EvenlopeAddr = m_IsChannel1 ? HW_NR12_SOUND_CHANNEL_1_VOL_ENVELOPE : HW_NR22_SOUND_CHANNEL_2_VOL_ENVELOPE;
		uint16_t m_SweepAddr = m_IsChannel1 ? HW_NR10_SOUND_CHANNEL_1_SWEEP : 0;
	};
}