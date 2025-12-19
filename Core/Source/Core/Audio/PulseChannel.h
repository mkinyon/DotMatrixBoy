#pragma once

#include "../Mmu.h"
#include "../Defines.h"
#include "Envelope.h"
#include "Length.h"

namespace Core
{
	class PulseChannel
	{
	public:
		PulseChannel(Mmu& mmu, bool isChannel1);
		~PulseChannel();

		void Clock();
		void LengthClock();
		void SweepClock();
		void EnvelopeClock();
		void Trigger();
		uint8_t GetCurrentSample();

	private:
		void UpdateSample();
		uint16_t GetFrequency() const;
		void SetFrequency(uint16_t frequency);

	private:
		Mmu& m_MMU;
		const bool m_DutyCycleTable[4][8] = {
		{0, 0, 0, 0, 0, 0, 0, 1},  // 12.5% duty cycle
		{0, 0, 0, 0, 0, 0, 1, 1},  // 25% duty cycle
		{0, 0, 0, 0, 1, 1, 1, 1},  // 50% duty cycle
		{0, 0, 1, 1, 1, 1, 1, 1}   // 75% duty cycle
		};

		bool m_IsActive = false;
		uint8_t m_CurrentSample = 0;

		uint8_t m_SelectedDuty = 0;
		uint16_t m_SampleIndex = 0;
		uint16_t m_CycleSampleUpdate = 0;
		uint16_t m_CurrentFrequency = 0;
		uint16_t m_CycleCount = 0;

		uint8_t m_SweepShift = 0;
		uint8_t m_SweepTime = 0;
		uint8_t m_ElaspsedSweepTime = 0;
		bool m_IsSweepDecreasing = false;

		// channel 1 & 2 parameters
		bool m_HasSweep = false;
		uint8_t  m_SoundControlFlag = 0;
		uint16_t m_DataAddr = 0;
		uint16_t m_FreqLowAddr = 0;
		uint16_t m_LengthDutyAddr = 0;
		uint16_t m_EvenlopeAddr = 0;
		uint16_t m_SweepAddr = 0;

		// components
		Envelope m_EnvelopeComp;
		Length m_LengthComp;
	};
}