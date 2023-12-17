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
		Cpu();
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
		int m_cycles = 0; // how many cycles remain before the cpu can fetch another instruction
		std::string currentInstructionName;
		bool m_interruptMasterFlag = false;
		bool m_isHalted = false;
		enum Cpu_Flags;

	public:
		void Reset(GameBoy& gb, bool enableBootRom);
		void Clock(GameBoy& gb);
		int Disassemble(uint8_t* opcode, int pc);
		std::map<uint16_t, std::string> DisassebleAll(GameBoy& gb);
		std::string GetCurrentInstruction();

	private:
		void process16bitInstruction(GameBoy& gb, uint16_t opcode, Cpu::m_CpuState& state);
		void processInterrupts(GameBoy& gb);

		// disassembly
		void disasseble16bit(uint8_t* opcode, int pc);
		void outputDisassembledInstruction(const char* instructionName, int pc, uint8_t* opcode, int totalOpBytes);
		void unimplementedInstruction(Cpu::m_CpuState& state, uint8_t opcode);

		// 8-bit Load Instructions
		void instruction_ld_reg_value(uint8_t& reg, uint8_t& value);
		void instruction_ld_addr_reg(GameBoy& gb, uint16_t& address, uint8_t& reg);
		void instruction_ld_reg_addr(GameBoy& gb, uint8_t& reg, uint16_t& address);

		// 16-bit Load Instructions
		void instruction_ld16_reg_value(uint16_t& reg, uint16_t value);


		// 8-bit Arithmetic/Logical Instructions
		void instruction_inc_reg(uint8_t& reg);
		void instruction_inc_hl(GameBoy& gb);

		void instruction_dec_reg(uint8_t& reg);
		void instruction_dec_hl(GameBoy& gb);

		void instruction_add_reg(uint8_t& reg);
		void instruction_add_hl(GameBoy& gb);

		void instruction_adc_reg(uint8_t& reg);
		void instruction_adc_hl(GameBoy& gb);

		void instruction_sub_reg(uint8_t& reg);
		void instruction_sub_hl(GameBoy& gb);

		void instruction_sbc_reg(uint8_t& reg);
		void instruction_sbc_hl(GameBoy& gb);

		void instruction_and_reg(uint8_t& reg);
		void instruction_and_hl(GameBoy& gb);

		void instruction_xor_reg(uint8_t& reg);
		void instruction_xor_hl(GameBoy& gb);

		void instruction_or_reg(uint8_t& reg);
		void instruction_or_hl(GameBoy& gb);

		void instruction_cp_reg(uint8_t& reg);
		void instruction_cp_hl(GameBoy& gb);


		// 16-bit Arithmetic/Logical Instructions
		void instruction_inc_reg16(uint16_t& reg);
		void instruction_dec_reg16(uint16_t& reg);
		void instruction_add_reg16(uint16_t& reg);
		void instruction_add_sp_e8(GameBoy& gb, uint8_t& e8);


		// 16 bit prefix (0xCB) instructions
		void instruction_rlc_reg(uint8_t& reg);
		void instruction_rlc_hl(GameBoy& gb);

		void instruction_rrc_reg(uint8_t& reg);
		void instruction_rrc_hl(GameBoy& gb);

		void instruction_rl_reg(uint8_t& reg);
		void instruction_rl_hl(GameBoy& gb);

		void instruction_rr_reg(uint8_t& reg);
		void instruction_rr_hl(GameBoy& gb);

		void instruction_sla_reg(uint8_t& reg);
		void instruction_sla_hl(GameBoy& gb);

		void instruction_sra_reg(uint8_t& reg);
		void instruction_sra_hl(GameBoy& gb);

		void instruction_swap_reg(uint8_t& reg);
		void instruction_swap_hl(GameBoy& gb);

		void instruction_srl_reg(uint8_t& reg);
		void instruction_srl_hl(GameBoy& gb);

		void instruction_bit_bit_reg(uint8_t& reg, uint8_t bit);
		void instruction_bit_bit_hl(GameBoy& gb, uint8_t bit);

		void instruction_res_bit_reg(uint8_t& reg, uint8_t bit);
		void instruction_res_bit_hl(GameBoy& gb, uint8_t bit);

		void instruction_set_bit_reg(uint8_t& reg, uint8_t bit);
		void instruction_set_bit_hl(GameBoy& gb, uint8_t bit);

		void pushSP(GameBoy& gb, uint16_t value);
		uint16_t popSP(GameBoy& gb);

		bool getCPUFlag(int flag);
		void setCPUFlag(int flag, bool enable);
	};

}