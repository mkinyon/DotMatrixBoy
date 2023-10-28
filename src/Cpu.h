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
	struct cpuFlags;
	struct cpuState;

public:
	void Reset(GameBoy& gb);
	void Clock(GameBoy& gb);
	int Disassemble(uint8_t *opcode, int pc);
};

void disasseble16bit(uint8_t *opcode, int pc);

void outputDisassembledInstruction(const char* instructionName, int pc, uint8_t *opcode, int totalOpBytes);

void unimplementedInstruction(Cpu::cpuState &state, uint8_t opcode);
