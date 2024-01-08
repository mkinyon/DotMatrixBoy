#pragma once

#include "..\Mmu.h"

namespace Core
{
	class SquareChannel
	{
	public:
		SquareChannel(Mmu& mmu);
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
	};
}