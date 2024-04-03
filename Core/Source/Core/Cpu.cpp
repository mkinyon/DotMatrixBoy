#include "Cpu.h"
#include "Defines.h"
#include "Logger.h"
#include "Utils.h"

#include <fstream>
#include <filesystem>
#include <sstream>

namespace Core
{
	Cpu::Cpu(IMmu& mmu) : m_MMU(mmu)
	{
		// Check if the log file already exists
		if (std::filesystem::exists("cpu.txt") && m_EnableLogging)
		{
			// If it exists, delete it
			std::filesystem::remove("cpu.txt");
		}
	}

	Cpu::~Cpu() {}

	int linecount = 0;
	std::ostringstream logBuffer;

	void Cpu::Clock()
	{
		Cpu::m_TotalCycles++;

		ProcessTimers();

		// Each instruction takes a certain amount of cycles to complete so
		// if there are still cycles remaining then we shoud just decrement 
		// the cycles and return;
		m_Cycles--;
		if (m_Cycles > 0)
		{
			m_InstructionCompleted = false;
			return;
		}

		// read opcode from memory
		uint8_t* opcode = &m_MMU.Read(m_State.PC);

		if (m_EnableLogging)
		{
			linecount++;
			logBuffer << DisassembleInstruction(opcode);

			if (linecount >= 10000)
			{
				std::ofstream outFile("cpu.txt", std::ios::out | std::ios::app);
				outFile << logBuffer.str();
				logBuffer.str("");  // Clear the buffer
				linecount = 0;      // Reset line count
				outFile.close();
			}
		}

		if (!m_IsHalted)
		{
			if (opcode[0] != 0xCB)
			{
				CurrentInstruction.definition = m_InstructionSet.m_InstructionMap[m_MMU.Read(m_State.PC)];
			}
			else
			{
				CurrentInstruction.definition = m_InstructionSet.m_16BitInstructionMap[m_MMU.Read(m_State.PC + 1)];
			}

			CurrentInstruction.lowByte = m_MMU.Read(m_State.PC + 1);
			CurrentInstruction.highByte = m_MMU.Read(m_State.PC + 2);

			m_State.PC += CurrentInstruction.definition.length;
			m_Cycles = CurrentInstruction.definition.instructionFunc(this);
		}
	
		ProcessInterrupts();
		m_InstructionCompleted = true;
	}

	void Cpu::ProcessInterrupts()
	{
		uint16_t destinationAddress = 0;
		uint8_t interruptFlag = 0;

		if (m_MMU.ReadRegisterBit(HW_FFFF_INTERRUPT_ENABLE, IE_VBLANK) &&
			m_MMU.ReadRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, IF_VBLANK))
		{
			destinationAddress = DEST_ADDRESS_VBLANK;
			interruptFlag = IF_VBLANK;
			m_IsHalted = false;
		}
		else if (m_MMU.ReadRegisterBit(HW_FFFF_INTERRUPT_ENABLE, IE_LCD) &&
			m_MMU.ReadRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, IF_LCD))
		{
			destinationAddress = DEST_ADDRESS_LCD_STAT;
			interruptFlag = IF_LCD;
			m_IsHalted = false;
		}
		else if (m_MMU.ReadRegisterBit(HW_FFFF_INTERRUPT_ENABLE, IE_TIMER) &&
			m_MMU.ReadRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, IF_TIMER))
		{
			destinationAddress = DEST_ADDRESS_TIMER;
			interruptFlag = IF_TIMER;
			m_IsHalted = false;
		}
		else if (m_MMU.ReadRegisterBit(HW_FFFF_INTERRUPT_ENABLE, IE_SERIAL) &&
			m_MMU.ReadRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, IF_SERIAL))
		{
			destinationAddress = DEST_ADDRESS_SERIAL;
			interruptFlag = IF_SERIAL;
			m_IsHalted = false;
		}
		else if (m_MMU.ReadRegisterBit(HW_FFFF_INTERRUPT_ENABLE, IE_JOYPAD) &&
			m_MMU.ReadRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, IF_JOYPAD))
		{
			destinationAddress = DEST_ADDRESS_JOYPAD;
			interruptFlag = IF_JOYPAD;
			m_IsHalted = false;
		}

		if (destinationAddress != 0 && m_State.IME)
		{
			m_State.IME = false;;
			m_MMU.WriteRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, interruptFlag, false);

			PushSP(m_State.PC);
			m_State.PC = destinationAddress;

			m_Cycles = 20;

			std::ostringstream stream;
			stream << "IRQ Write - Address: " << std::hex << destinationAddress << " Interrupt Type: ";
			Logger::Instance().Verbose(Domain::CPU, stream.str());
		}
	}

	std::map<uint16_t, std::string> Cpu::DisassebleAll()
	{
		std::map<uint16_t, std::string> mapLines;
		uint16_t pc = 0;

		while (pc < 0x7FFF)
		{
			uint8_t* opcode = &m_MMU.Read(pc);
			int nextPC;

			if (opcode[0] != 0xCB)
			{
				nextPC = instrSet.m_InstructionMap[opcode[0]].length;
			}
			else
			{
				nextPC = 2;
			}

			mapLines[pc] = DisassembleInstruction(opcode);
			pc += nextPC;
		}

		return mapLines;
	}

	std::string Cpu::GetCurrentInstruction()
	{
		return m_CurrentInstructionName;
	}

	void Cpu::Reset(bool enableBootRom)
	{
		Cpu::m_TotalCycles = 0;
		m_IsHalted = false;

		// registers
		m_State.AF = enableBootRom ? 0x0000 : 0x01B0; // flags - should be reset to $B0
		m_State.BC = enableBootRom ? 0x0000 : 0x0013;
		m_State.DE = enableBootRom ? 0x0000 : 0x00D8;
		m_State.HL = enableBootRom ? 0x0000 : 0x014D;
		m_State.PC = enableBootRom ? 0x0000 : 0x100; // game boy execution start point
		m_State.SP = enableBootRom ? 0x0000 : 0xFFFE;
		m_State.IME = false;

		// hardware registers
		m_MMU.Write(0xFF00, 0xCF);
		m_MMU.Write(0xFF01, 0x00);
		m_MMU.Write(0xFF02, 0x7E);
		//m_MMU.Write(0xFF04, 0xAB);
		m_MMU.Write(0xFF05, 0x00);
		m_MMU.Write(0xFF06, 0x00);
		m_MMU.Write(0xFF07, 0x00);
		m_MMU.Write(0xFF0F, 0x00);
		m_MMU.Write(0xFF10, 0x80);
		m_MMU.Write(0xFF11, 0xBF);
		m_MMU.Write(0xFF12, 0xF3);
		m_MMU.Write(0xFF13, 0xFF);
		m_MMU.Write(0xFF14, 0xBF);
		m_MMU.Write(0xFF16, 0x3F);
		m_MMU.Write(0xFF17, 0x00);
		m_MMU.Write(0xFF18, 0xFF);
		m_MMU.Write(0xFF19, 0xBF);
		m_MMU.Write(0xFF1A, 0x7F);
		m_MMU.Write(0xFF1B, 0xFF);
		m_MMU.Write(0xFF1C, 0x9F);
		m_MMU.Write(0xFF1D, 0xFF);
		m_MMU.Write(0xFF1E, 0xBF);
		m_MMU.Write(0xFF20, 0xFF);
		m_MMU.Write(0xFF21, 0x00);
		m_MMU.Write(0xFF22, 0x00);
		m_MMU.Write(0xFF23, 0xBF);
		m_MMU.Write(0xFF24, 0x77);
		m_MMU.Write(0xFF25, 0xF3);
		m_MMU.Write(0xFF26, 0xF1);
		m_MMU.Write(0xFF40, 0x91);
		m_MMU.Write(0xFF41, 0x85);
		m_MMU.Write(0xFF42, 0x00);
		m_MMU.Write(0xFF43, 0x00);
		m_MMU.Write(0xFF44, 0x00);
		m_MMU.Write(0xFF45, 0x00);
		m_MMU.Write(0xFF46, 0xFF);
		m_MMU.Write(0xFF47, 0xFC);
		m_MMU.Write(0xFF48, 0xFF);
		m_MMU.Write(0xFF49, 0xFF);
		m_MMU.Write(0xFF4A, 0x00);
		m_MMU.Write(0xFF4B, 0x00);
		m_MMU.Write(HW_FFFF_INTERRUPT_ENABLE, 0x00);
	}

	void Cpu::PushSP(uint16_t value)
	{
		m_MMU.Write(--m_State.SP, (value >> 8) & 0xFF);
		m_MMU.Write(--m_State.SP, value & 0xFF);
	}

	uint16_t Cpu::PopSP()
	{
		uint8_t firstByte = m_MMU.Read(m_State.SP++);
		uint8_t secondByte = m_MMU.Read(m_State.SP++);
		return (secondByte << 8) | (firstByte);
	}

	bool Cpu::GetCPUFlag(int flag)
	{
		return (m_State.F & flag) != 0;
	}

	void Cpu::SetCPUFlag(int flag, bool enable)
	{
		if (enable)
			m_State.F |= flag;
		else
			m_State.F &= ~flag;
	}

	void Cpu::ProcessTimers() 
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

	Cpu::sCPUState* Cpu::GetState()
	{
		return &m_State;
	}

	void Cpu::SetState(sCPUState state)
	{
		m_State = state;
	}

	IMmu& Cpu::GetMMU()
	{
		return m_MMU;
	}

	std::string Cpu::DisassembleInstruction(uint8_t* opcode)
	{
		uint8_t numOfBytes = instrSet.m_InstructionMap[opcode[0]].length;
		std::ostringstream instrBuffer;

		instrBuffer << std::hex << std::setfill('0') << std::uppercase <<
			"A:" << std::setw(2) << static_cast<int>(m_State.A) <<
			" F:" << (GetCPUFlag(FLAG_ZERO) ? "Z" : "-") <<
			(GetCPUFlag(FLAG_SUBTRACT) ? "N" : "-") <<
			(GetCPUFlag(FLAG_HALF_CARRY) ? "H" : "-") <<
			(GetCPUFlag(FLAG_CARRY) ? "C" : "-") <<
			" BC:" << std::setw(4) << static_cast<int>(m_State.BC) << std::nouppercase <<
			" DE:" << std::setw(4) << static_cast<int>(m_State.DE) <<
			" HL:" << std::setw(4) << static_cast<int>(m_State.HL) <<
			" SP:" << std::setw(4) << static_cast<int>(m_State.SP) <<
			" PC:" << std::setw(4) << static_cast<int>(m_State.PC) <<
			" (cy: " << std::dec << static_cast<int>(m_TotalCycles) - 1 << ")" << std::hex <<
			" ppu:+" << static_cast<int>(m_MMU.Read(HW_FF41_STAT_LCD_STATUS) & 0b11) <<
			" |[00]0x" << std::setw(4) << static_cast<int>(m_State.PC) << ":" <<
			std::setw(2) << static_cast<int>(opcode[0]) << " ";

		if (numOfBytes > 1)
		{
			instrBuffer << std::setw(2) << static_cast<int>(opcode[1]) << " ";
		}
		else
		{
			instrBuffer << "   ";
		}

		if (numOfBytes > 2)
		{
			instrBuffer << std::setw(2) << static_cast<int>(opcode[2]) << "";
		}
		else
		{
			instrBuffer << "   ";
		}

		if (opcode[0] != 0xCB)
		{
			instrBuffer << "  " << instrSet.m_InstructionMap[opcode[0]].nmemonic;
		}
		else
		{
			instrBuffer << "  " << instrSet.m_16BitInstructionMap[opcode[1]].nmemonic;
		}

		instrBuffer << std::endl;

		return instrBuffer.str();
	}
}