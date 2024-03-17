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
		uint16_t GetFrequency() const;
		void SetFrequency(uint16_t frequency);

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


		uint8_t sweepShift = 0;
		uint8_t sweepTime = 0;
		uint8_t elaspsedSweepTime = 0;
		bool isSweepDecreasing = false;

		// channel 1 & 2 parameters
		bool m_HasSweep = false;
		uint8_t  m_SoundControlFlag = 0;
		uint16_t m_DataAddr = 0;
		uint16_t m_FreqLowAddr = 0;
		uint16_t m_LengthDutyAddr = 0;
		uint16_t m_EvenlopeAddr = 0;
		uint16_t m_SweepAddr = 0;
	};
}