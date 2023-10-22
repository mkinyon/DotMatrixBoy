#pragma once
#include <stdint.h>
#include <iostream>
#include <vector>

class Cpu
{
public:
	Cpu();
	~Cpu();

private:
	// 8 bit registers
	// Note: Some instructions can pair two registers as 16 bit registers.
	//  The pairings are AF, BC, DE, & HL.

	uint8_t a = 0x00;
	uint8_t b = 0x00;
	uint8_t c = 0x00;
	uint8_t d = 0x00;
	uint8_t e = 0x00;
	uint8_t f = 0x00;
	uint8_t h = 0x00;

public:
	// 16 bit registers
	uint16_t sp = 0x00; // stack pointer
	uint16_t pc = 0x100; // the gameboy program counter starts at $100

private:
	// Flag register
	// 7th bit: Zero flag (Z)
	// 6th bit: Subtract flag (N)
	// 5th bit: Half Carry Flag (H)
	// 4th bit: Carry flag (C)
	// 3rd bit: not used (0)
	// 2nd bit: not used (0)
	// 1st bit: not used (0)
	// 0 bit: not used (0)
	uint8_t flag = 0x00;
	

	uint8_t opcode = 0x00;

public:
	void clock( uint8_t opcode );
	uint8_t read(uint8_t address);
	int disassemble(std::vector<uint8_t> *rom, int pc);
};

void disasseble16bit(std::vector<uint8_t>* rom, uint16_t opcode, int pc);
void outputDisassembledInstruction(const char* instructionName, int pc, std::vector<uint8_t>* rom, int totalOpBytes);

