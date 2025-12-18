
#pragma once
#include "Mmu.h"
#include "BaseDevice.h"

namespace Core
{
	class Timer : public BaseDevice
	{
	public:
		Timer(Mmu& mmu);
		~Timer();

		void Clock();
		void OnWrite(uint16_t address, uint8_t value) override;

	private:
		Mmu& m_MMU;
		
		// Internal 16-bit DIV counter (not directly accessible, only upper 8 bits at 0xFF04)
		uint16_t m_InternalDIVCounter;
		
		// Last bit value for edge detection
		bool m_LastBit;
		
		// Countdown for TIMA overflow interrupt delay
		int m_CountdownToInterrupt;
	};
}