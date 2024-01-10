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
		uint8_t CurrentSample = 0;

	private:
		void updateSample();
		uint16_t getFrequency();

	private:
		Mmu& mmu;
		const bool dutyCycleTable[4][8] = {
		{0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 1, 1, 1},
		{0, 1, 1, 1, 1, 1, 1, 0}
		};

		bool isActive = 0;
		uint8_t selectedDuty = 0;
		uint16_t sampleIndex = 0;
		uint16_t cycleSampleUpdate = 0;
		uint16_t currentFrequency = 0;
		uint16_t cycleCount = 0;

		bool lengthStop = false;
		uint16_t length = 0;

		// channel 1 & 2 parameters
		bool isChannel1;
		bool hasSweep = isChannel1;
		uint8_t  soundControlFlag = isChannel1 ?  NR52_CH1_ON: NR52_CH2_ON;
		uint16_t dataAddr = isChannel1 ? HW_NR14_SOUND_CHANNEL_1_PERIOD_HIGH : HW_NR24_SOUND_CHANNEL_2_PERIOD_HIGH;
		uint16_t freqLowAddr = isChannel1 ? HW_NR13_SOUND_CHANNEL_1_PERIOD_LOW : HW_NR13_SOUND_CHANNEL_1_PERIOD_LOW;
		uint16_t lengthDutyAddr = isChannel1 ? HW_NR11_SOUND_CHANNEL_1_LEN_TIMER : HW_NR21_SOUND_CHANNEL_2_LEN_TIMER;
		uint16_t evenlopeAddr = isChannel1 ? HW_NR12_SOUND_CHANNEL_1_VOL_ENVELOPE : HW_NR22_SOUND_CHANNEL_2_VOL_ENVELOPE;
		uint16_t sweepAddr = isChannel1 ? HW_NR10_SOUND_CHANNEL_1_SWEEP : 0;
	};
}