#pragma once
#include <stdint.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

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

			uint16_t AF;
		};

		union
		{
			struct
			{
				uint8_t C;
				uint8_t B;
			};

			uint16_t BC;
		};

		union
		{
			struct
			{
				uint8_t E;
				uint8_t D;
			};

			uint16_t DE;
		};

		union
		{
			struct
			{
				uint8_t L;
				uint8_t H;
			};

			uint16_t HL;
		};

		uint16_t SP = 0x00; // stack pointer
		uint16_t PC = 0x100; // the gameboy program counter starts at $100

		uint8_t	int_enable;
	} State;
	int TotalCycles = 0;

public:
	void Reset(GameBoy& gb);
	void Clock(GameBoy& gb);
	int Disassemble(uint8_t *opcode, int pc);

private:
	void disasseble16bit(uint8_t *opcode, int pc);
	void outputDisassembledInstruction(const char* instructionName, int pc, uint8_t *opcode, int totalOpBytes);
	void unimplementedInstruction(Cpu::m_CpuState &state, uint8_t opcode);

	enum Flags;

	bool getFlag(Flags flag);
	void setFlag(Flags flag);
	void clearFlag(Flags flag);

	void pushSP(GameBoy& gb, uint16_t value);
	uint16_t popSP(GameBoy& gb);
};
