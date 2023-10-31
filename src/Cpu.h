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
	struct m_CpuState;

public:
	void Reset(GameBoy& gb);
	void Clock(GameBoy& gb);
	int Disassemble(uint8_t *opcode, int pc);
};

void disasseble16bit(uint8_t *opcode, int pc);
void outputDisassembledInstruction(const char* instructionName, int pc, uint8_t *opcode, int totalOpBytes);
void unimplementedInstruction(Cpu::m_CpuState &state, uint8_t opcode);

enum Flags;

bool getFlag(Flags flag);
void setFlag(Flags flag);
void clearFlag(Flags flag);

void pushSP(GameBoy& gb, uint16_t value);
uint16_t popSP(GameBoy& gb);
