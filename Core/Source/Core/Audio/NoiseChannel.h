#pragma once

#include "../Mmu.h"
#include "../Defines.h"
#include "Envelope.h"
#include "Length.h"

namespace Core
{
	class NoiseChannel
	{
	public: 
		NoiseChannel(Mmu& mmu);
		~NoiseChannel();

		void Trigger();
		void Clock();
		void LengthClock();
		void EnvelopeClock();

		uint8_t GetCurrentSample();

	private:
		void UpdateSample();

	private:
		Mmu& m_MMU;
		
		bool m_IsActive = false;
		uint8_t m_CurrentSample = 0;

		uint16_t m_SampleIndex = 0;
		uint16_t m_CurrentFrequency = 0;

		const uint8_t m_AudioDivisorArray[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };
		bool m_IsWidth7Bit = false;
		uint8_t m_Divisor = 0;
		uint16_t m_lfsr = 0x7FFF;
		uint16_t m_CycleSampleUpdate = 0;
		uint16_t m_CycleCount = 0;
		
		// components
		Envelope m_EnvelopeComp;
		Length m_LengthComp;

	};
}