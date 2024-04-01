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

		int m_TotalCycles = 0;
		bool m_InstructionCompleted = false;
		bool m_EnableLogging = true;
		
	private:
		sCPUState m_State;
		InstructionSet instrSet;

		IMmu& m_MMU;
		int m_Cycles = 0; // how many cycles remain before the cpu can fetch another instruction
		std::string m_CurrentInstructionName;
		bool m_IsHalted = false;
		enum Cpu_Flags;

	public:
		void Reset(bool enableBootRom);
		void Clock();

		std::map<uint16_t, std::string> DisassebleAll();
		std::string GetCurrentInstruction();

		bool GetCPUFlag(int flag);
		void SetCPUFlag(int flag, bool enable);

		sCPUState* GetState();
		void SetState(sCPUState state);

		IMmu& GetMMU();

		InstructionSet::sInstruction CurrentInstruction;

	private:
		std::string DisassembleInstruction(uint8_t* opcode);

		void Process16bitInstruction(uint16_t opcode, sCPUState& state);
		void ProcessInterrupts();
		void ProcessTimers();
		void PushSP(uint16_t value);
		uint16_t PopSP();


		//// Instructions ////
		// 8-bit Load Instructions
		void Instruction_ld_reg_value(uint8_t& reg, uint8_t& value);
		void Instruction_ld_addr_reg(uint16_t& address, uint8_t& reg);
		void Instruction_ld_reg_addr(uint8_t& reg, uint16_t& address);


		// 16-bit Load Instructions
		void Instruction_ld16_reg_value(uint16_t& reg, uint16_t value);


		// 8-bit Arithmetic/Logical Instructions
		void Instruction_inc_reg(uint8_t& reg);
		void Instruction_inc_hl();

		void Instruction_dec_reg(uint8_t& reg);
		void Instruction_dec_hl();

		void Instruction_add_reg(uint8_t& reg);
		void Instruction_add_hl();

		void Instruction_adc_reg(uint8_t& reg);
		void Instruction_adc_hl();

		void Instruction_sub_reg(uint8_t& reg);
		void Instruction_sub_hl();

		void Instruction_sbc_reg(uint8_t& reg);
		void Instruction_sbc_hl();

		void Instruction_and_reg(uint8_t& reg);
		void Instruction_and_hl();

		void Instruction_xor_reg(uint8_t& reg);
		void Instruction_xor_hl();

		void Instruction_or_reg(uint8_t& reg);
		void Instruction_or_hl();

		void Instruction_cp_reg(uint8_t& reg);
		void Instruction_cp_hl();


		// 16-bit Arithmetic/Logical Instructions
		void Instruction_inc_reg16(uint16_t& reg);
		void Instruction_dec_reg16(uint16_t& reg);
		void Instruction_add_reg16(uint16_t& reg);
		void Instruction_add_sp_e8(uint8_t& e8);


		// 16 bit prefix (0xCB) instructions
		void Instruction_rlc_reg(uint8_t& reg);
		void Instruction_rlc_hl();

		void Instruction_rrc_reg(uint8_t& reg);
		void Instruction_rrc_hl();

		void Instruction_rl_reg(uint8_t& reg);
		void Instruction_rl_hl();

		void Instruction_rr_reg(uint8_t& reg);
		void Instruction_rr_hl();

		void Instruction_sla_reg(uint8_t& reg);
		void Instruction_sla_hl();

		void Instruction_sra_reg(uint8_t& reg);
		void Instruction_sra_hl();

		void Instruction_swap_reg(uint8_t& reg);
		void Instruction_swap_hl();

		void Instruction_srl_reg(uint8_t& reg);
		void Instruction_srl_hl();

		void Instruction_bit_bit_reg(uint8_t& reg, uint8_t bit);
		void Instruction_bit_bit_hl(uint8_t bit);

		void Instruction_res_bit_reg(uint8_t& reg, uint8_t bit);
		void Instruction_res_bit_hl(uint8_t bit);

		void Instruction_set_bit_reg(uint8_t& reg, uint8_t bit);
		void Instruction_set_bit_hl(uint8_t bit);
	};
}