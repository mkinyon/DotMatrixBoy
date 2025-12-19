#pragma once

#include "../Mmu.h"
#include "../Defines.h"
#include "Length.h"

namespace Core
{
	class WaveChannel
	{
	public:
		WaveChannel(Mmu& mmu);
		~WaveChannel();

		void Start();
		void Stop();
		void Trigger();
		void LengthClock();
		void Clock();

		uint8_t GetCurrentSample() const;

	private:
		uint16_t GetFrequency() const;
		void UpdateSample();

		Mmu& m_MMU;

		bool m_IsActive = false;
		bool m_IsRunning = false;
		uint8_t m_CurrentSample = 0;

		uint16_t m_SampleIndex = 0;
		uint16_t m_CycleSampleUpdate = 0;
		uint16_t m_CycleCount = 0;

		// components
		Length m_LengthComp;
	};
}