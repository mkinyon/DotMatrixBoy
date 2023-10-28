#pragma once
#include <stdint.h>
#include <vector>

#include "Cpu.h"
#include "Cartridge.h"

class GameBoy
{
public:
	Cpu cpu;

	GameBoy();
	~GameBoy();

public:
	void Run();
	void InsertCartridge(Cartridge &cart);
	uint8_t& ReadFromMemoryMap(uint16_t address);
	void WriteToMemoryMap(uint16_t address, uint8_t value);

private:
	//devices
	Cartridge cart;
	uint8_t ram[4095];
	uint8_t zeroPage[127]; // $FF80-$FFFE  Zero Page - 127 bytes
};

