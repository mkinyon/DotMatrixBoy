#pragma once

#include <stdint.h>
#include <iostream>
#include <vector>
#include <map>

namespace Core
{
	class GameBoy;

	class Cpu
	{
	public:
		Cpu(GameBoy& gb);
		~Cpu();

	public:
		struct m_CpuState
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
		} State;
		int m_TotalCycles = 0;
		bool m_InstructionCompleted = false;

	private:
		GameBoy& gb;
		int m_cycles = 0; // how many cycles remain before the cpu can fetch another instruction
		std::string currentInstructionName;
		bool m_interruptMasterFlag = false;
		bool m_isHalted = false;
		enum Cpu_Flags;

	public:
		void Reset(bool enableBootRom);
		void Clock();
		int Disassemble(uint8_t* opcode, int pc);
		std::map<uint16_t, std::string> DisassebleAll();
		std::string GetCurrentInstruction();

		bool GetCPUFlag(int flag);
		void SetCPUFlag(int flag, bool enable);

	private:
		void process16bitInstruction(uint16_t opcode, Cpu::m_CpuState& state);
		void processInterrupts();

		// disassembly
		void disasseble16bit(uint8_t* opcode, int pc);
		void outputDisassembledInstruction(const char* instructionName, int pc, uint8_t* opcode, int totalOpBytes);
		void unimplementedInstruction(Cpu::m_CpuState& state, uint8_t opcode);

		// 8-bit Load Instructions
		void instruction_ld_reg_value(uint8_t& reg, uint8_t& value);
		void instruction_ld_addr_reg(uint16_t& address, uint8_t& reg);
		void instruction_ld_reg_addr(uint8_t& reg, uint16_t& address);

		// 16-bit Load Instructions
		void instruction_ld16_reg_value(uint16_t& reg, uint16_t value);


		// 8-bit Arithmetic/Logical Instructions
		void instruction_inc_reg(uint8_t& reg);
		void instruction_inc_hl();

		void instruction_dec_reg(uint8_t& reg);
		void instruction_dec_hl();

		void instruction_add_reg(uint8_t& reg);
		void instruction_add_hl();

		void instruction_adc_reg(uint8_t& reg);
		void instruction_adc_hl();

		void instruction_sub_reg(uint8_t& reg);
		void instruction_sub_hl();

		void instruction_sbc_reg(uint8_t& reg);
		void instruction_sbc_hl();

		void instruction_and_reg(uint8_t& reg);
		void instruction_and_hl();

		void instruction_xor_reg(uint8_t& reg);
		void instruction_xor_hl();

		void instruction_or_reg(uint8_t& reg);
		void instruction_or_hl();

		void instruction_cp_reg(uint8_t& reg);
		void instruction_cp_hl();


		// 16-bit Arithmetic/Logical Instructions
		void instruction_inc_reg16(uint16_t& reg);
		void instruction_dec_reg16(uint16_t& reg);
		void instruction_add_reg16(uint16_t& reg);
		void instruction_add_sp_e8(uint8_t& e8);


		// 16 bit prefix (0xCB) instructions
		void instruction_rlc_reg(uint8_t& reg);
		void instruction_rlc_hl();

		void instruction_rrc_reg(uint8_t& reg);
		void instruction_rrc_hl();

		void instruction_rl_reg(uint8_t& reg);
		void instruction_rl_hl();

		void instruction_rr_reg(uint8_t& reg);
		void instruction_rr_hl();

		void instruction_sla_reg(uint8_t& reg);
		void instruction_sla_hl();

		void instruction_sra_reg(uint8_t& reg);
		void instruction_sra_hl();

		void instruction_swap_reg(uint8_t& reg);
		void instruction_swap_hl();

		void instruction_srl_reg(uint8_t& reg);
		void instruction_srl_hl();

		void instruction_bit_bit_reg(uint8_t& reg, uint8_t bit);
		void instruction_bit_bit_hl(uint8_t bit);

		void instruction_res_bit_reg(uint8_t& reg, uint8_t bit);
		void instruction_res_bit_hl(uint8_t bit);

		void instruction_set_bit_reg(uint8_t& reg, uint8_t bit);
		void instruction_set_bit_hl(uint8_t bit);

		void pushSP(uint16_t value);
		uint16_t popSP();

		int getClockSelect();
		void processTimers();
	};

}