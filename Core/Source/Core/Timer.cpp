
#include "Timer.h"

namespace Core
{
	Timer::Timer(Mmu& mmu) : m_MMU(mmu), m_InternalDIVCounter(0), m_LastBit(false), m_CountdownToInterrupt(0)
	{
		m_MMU.RegisterOnWrite(this);
	}
	
	Timer::~Timer() {}

	void Timer::Clock()
	{
		// Increment internal DIV counter (16-bit, increments every T-cycle)
		m_InternalDIVCounter++;

		// Update the readable DIV register (only upper 8 bits at 0xFF04)
		m_MMU.Write(HW_FF04_DIV_DIVIDER_REGISTER, (m_InternalDIVCounter & 0xFF00) >> 8, true);

		// Timer increment logic based on DIV counter bit transitions
		// Reference: https://github.com/Hacktix/GBEDG/blob/master/timers/index.md#timer-operation
		bool thisBit = false;

		// 1. A bit position of the 16-bit counter is determined based on the lower 2 bits of the TAC register
		uint8_t tac = m_MMU.Read(HW_FF07_TAC_TIMER_CONTROL);
		switch (tac & 0x03)
		{
		case 0: // 4096 Hz (bit 9)
			thisBit = (m_InternalDIVCounter >> 9) & 0x1;
			break;
		case 1: // 262144 Hz (bit 3)
			thisBit = (m_InternalDIVCounter >> 3) & 0x1;
			break;
		case 2: // 65536 Hz (bit 5)
			thisBit = (m_InternalDIVCounter >> 5) & 0x1;
			break;
		case 3: // 16384 Hz (bit 7)
			thisBit = (m_InternalDIVCounter >> 7) & 0x1;
			break;
		}

		// 2. The "Timer Enable" bit (Bit 2) is extracted from the value in the TAC register
		bool timerEnabled = (tac & TAC_ENABLE) != 0;

		// 3. The bit taken from the DIV counter is ANDed with the Timer Enable bit
		//    The result of this operation will be referred to as the "AND Result"
		thisBit = thisBit && timerEnabled;

		// 4. Detect falling edge (1 -> 0 transition) to increment TIMA
		if (m_LastBit == true && thisBit == false)
		{
			// Increment TIMA register
			uint8_t tima = m_MMU.Read(HW_FF05_TIMA_TIMER_COUNTER);
			tima++;
			m_MMU.Write(HW_FF05_TIMA_TIMER_COUNTER, tima);

			// If TIMA overflows (0xFF -> 0x00), schedule interrupt after 4 cycles
			if (tima == 0x00)
			{
				m_CountdownToInterrupt = 4;
			}
		}

		// Handle TIMA overflow interrupt delay
		if (m_CountdownToInterrupt > 0)
		{
			m_CountdownToInterrupt--;
			if (m_CountdownToInterrupt == 0)
			{
				// Set timer interrupt flag
				m_MMU.WriteRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, IF_TIMER, true);
				// Reload TIMA from TMA
				m_MMU.Write(HW_FF05_TIMA_TIMER_COUNTER, m_MMU.Read(HW_FF06_TMA_TIMER_MODULO));
			}
		}

		// Store current bit for next cycle's edge detection
		m_LastBit = thisBit;
	}

	void Timer::OnWrite(uint16_t address, uint8_t value)
	{
		// Writing to DIV register (0xFF04) resets the internal counter to 0
		if (address == HW_FF04_DIV_DIVIDER_REGISTER)
		{
			m_InternalDIVCounter = 0;
			// Recalculate lastBit immediately after DIV reset
			// This is important because resetting DIV can affect the timer increment detection
			uint8_t tac = m_MMU.Read(HW_FF07_TAC_TIMER_CONTROL);
			bool timerEnabled = (tac & TAC_ENABLE) != 0;
			bool thisBit = false;
			
			switch (tac & 0x03)
			{
			case 0:
				thisBit = (m_InternalDIVCounter >> 9) & 0x1;
				break;
			case 1:
				thisBit = (m_InternalDIVCounter >> 3) & 0x1;
				break;
			case 2:
				thisBit = (m_InternalDIVCounter >> 5) & 0x1;
				break;
			case 3:
				thisBit = (m_InternalDIVCounter >> 7) & 0x1;
				break;
			}
			
			m_LastBit = thisBit && timerEnabled;
		}
		// Writing to TAC register can affect timer increment detection
		else if (address == HW_FF07_TAC_TIMER_CONTROL)
		{
			// Recalculate lastBit after TAC change
			bool timerEnabled = (value & TAC_ENABLE) != 0;
			bool thisBit = false;
			
			switch (value & 0x03)
			{
			case 0:
				thisBit = (m_InternalDIVCounter >> 9) & 0x1;
				break;
			case 1:
				thisBit = (m_InternalDIVCounter >> 3) & 0x1;
				break;
			case 2:
				thisBit = (m_InternalDIVCounter >> 5) & 0x1;
				break;
			case 3:
				thisBit = (m_InternalDIVCounter >> 7) & 0x1;
				break;
			}
			
			m_LastBit = thisBit && timerEnabled;
		}
	}
}