#pragma once

#include <stdint.h>
#include <iostream>
#include <vector>

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

private:
	int m_cycles = 0; // how many cycles remain before the cpu can fetch another instruction
	bool m_interruptMasterFlag = false;
	bool m_isHalted = false; 
	enum Cpu_Flags;

public:
	void Reset(GameBoy& gb, bool enableBootRom);
	void Clock(GameBoy& gb);
	int Disassemble(uint8_t *opcode, int pc);

private:
	void process16bitInstruction(GameBoy& gb, uint16_t opcode, Cpu::m_CpuState& state);
	void processInterrupts(GameBoy& gb);

	// disassembly
	void disasseble16bit(uint8_t *opcode, int pc);
	void outputDisassembledInstruction(const char* instructionName, int pc, uint8_t *opcode, int totalOpBytes);
	void unimplementedInstruction(Cpu::m_CpuState &state, uint8_t opcode);

	// 16 bit (0xCB) instructions
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
