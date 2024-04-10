
#include "Timer.h"

namespace Core
{
	Timer::Timer(Mmu& mmu) : m_MMU(mmu) {}
	Timer::~Timer() {}

	void Timer::Clock()
	{
		// increment DIV register
		uint16_t internalClock = (m_MMU.Read(HW_FF04_DIV_DIVIDER_REGISTER) << 8)
			| m_MMU.Read(HW_FF03_DIV_DIVIDER_REGISTER_LOW);

		internalClock++;

		// if the div clock rolls over then we need to copy the value of TIMA to TMA
		if (internalClock == 0xFFFF)
		{
			m_MMU.Write(HW_FF06_TMA_TIMER_MODULO, m_MMU.Read(HW_FF05_TIMA_TIMER_COUNTER));
		}

		// write updated DIV register
		m_MMU.Write(HW_FF04_DIV_DIVIDER_REGISTER, (internalClock & 0xFF00) >> 8, true);
		m_MMU.Write(HW_FF03_DIV_DIVIDER_REGISTER_LOW, internalClock & 0x00FF, true);


		// https://github.com/Hacktix/GBEDG/blob/master/timers/index.md#timer-operation
		static bool lastBit;
		bool thisBit = 0;

		// 1. A bit position of the 16 - bit counter is determined based on the lower 2 bits of the TAC register
		switch (m_MMU.Read(HW_FF07_TAC_TIMER_CONTROL) & 0x03)
		{
		case 0:
			thisBit = (internalClock >> 9) & 0x1;
			break;
		case 1:
			thisBit = (internalClock >> 3) & 0x1;
			break;
		case 2:
			thisBit = (internalClock >> 5) & 0x1;
			break;
		case 3:
			thisBit = (internalClock >> 7) & 0x1;
			break;
		}

		// 2. The "Timer Enable" bit(Bit 2) is extracted from the value in the TAC register and stored for the next step.
		bool timerEnabled = m_MMU.ReadRegisterBit(HW_FF07_TAC_TIMER_CONTROL, TAC_ENABLE);

		// 3. The bit taken from the DIV counter is ANDed with the Timer Enable bit. 
		//    The result of this operation will be referred to as the "AND Result".
		thisBit &= timerEnabled;

		static int countdownToInterrupt = 0;
		if (lastBit == 1 && thisBit == 0)
		{
			// now increment the TIMA register
			uint8_t tima = m_MMU.Read(HW_FF05_TIMA_TIMER_COUNTER);
			tima++;
			m_MMU.Write(HW_FF05_TIMA_TIMER_COUNTER, tima);

			// if the TIMA register rolls over then we need to trigger an interrupt
			if (tima == 0x0)
			{
				// need to wait four cycles plus this current cycle
				countdownToInterrupt = 5;
			}
		}

		if (countdownToInterrupt > 0)
		{
			countdownToInterrupt--;
			if (countdownToInterrupt == 0)
			{
				m_MMU.WriteRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, IF_TIMER, true);
				m_MMU.Write(HW_FF05_TIMA_TIMER_COUNTER, m_MMU.Read(HW_FF06_TMA_TIMER_MODULO));
			}
		}

		lastBit = thisBit;
	}
}