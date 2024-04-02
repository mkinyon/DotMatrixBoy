#pragma once

#include "IMmu.h"
#include "InstructionSet.h"

#include <stdint.h>
#include <iostream>
#include <vector>
#include <map>

namespace Core
{
	class Cpu
	{
	public:
		Cpu(IMmu& mmu);
		~Cpu();

	public:
		struct sCPUState
		{
			// 8 bit registers that are grouped into 16 bit pairs
			// The pairings are AF, BC, DE, & HL.
			union
			{
				struct
				{
					uint8_t F;
					uint8_t A; // this register is used for flags
				};

				uint16_t AF = 0x00;
			};

			union
			{
				struct
				{
					uint8_t C;
					uint8_t B;
				};

				uint16_t BC = 0x00;
			};

			union
			{
				struct
				{
					uint8_t E;
					uint8_t D;
				};

				uint16_t DE = 0x00;
			};

			union
			{
				struct
				{
					uint8_t L;
					uint8_t H;
				};

				uint16_t HL = 0x00;
			};

			uint16_t SP = 0x00; // stack pointer
			uint16_t PC = 0x100; // the gameboy program counter starts at $100
			bool IME = false; // interrupt master enable
		};

		bool m_IsHalted = false;

		int m_TotalCycles = 0;
		bool m_InstructionCompleted = false;
		bool m_EnableLogging = false;
		
	private:
		sCPUState m_State;
		InstructionSet instrSet;

		IMmu& m_MMU;
		int m_Cycles = 0; // how many cycles remain before the cpu can fetch another instruction
		std::string m_CurrentInstructionName;
		enum Cpu_Flags;

	public:
		void Reset(bool enableBootRom);
		void Clock();

		std::map<uint16_t, std::string> DisassebleAll();
		std::string GetCurrentInstruction();

		bool GetCPUFlag(int flag);
		void SetCPUFlag(int flag, bool enable);
		void PushSP(uint16_t value);
		uint16_t PopSP();

		sCPUState* GetState();
		void SetState(sCPUState state);

		IMmu& GetMMU();

		InstructionSet::sInstruction CurrentInstruction;
		InstructionSet m_InstructionSet;

	private:
		std::string DisassembleInstruction(uint8_t* opcode);

		void ProcessInterrupts();
		void ProcessTimers();
	};
}