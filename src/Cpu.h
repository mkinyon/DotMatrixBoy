#pragma once
#include <stdint.h>
#include <iostream>
#include <vector>

class Cpu
{
public:
	Cpu();
	~Cpu();

public:
	struct cpuFlags;
	struct cpuState;

public:
	void loadCartData(std::vector<uint8_t>* rom);
	void clock();
	int disassemble(std::vector<uint8_t> *rom, int pc);
};



void disasseble16bit(std::vector<uint8_t>* rom, uint16_t opcode, int pc);
void outputDisassembledInstruction(const char* instructionName, int pc, std::vector<uint8_t>* rom, int totalOpBytes);

void unimplementedInstruction(Cpu::cpuState& state, uint8_t opcode);
