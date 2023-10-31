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
	uint8_t ram[4095];           // $C000-$CFFF   Internal RAM - Bank 0
	uint8_t bgMapData2[1023];    // $9C00-$9FFF   BG Map Data 2
	uint8_t bgMapData1[1023];    // $9800-$9BFF   BG Map Data 1
	uint8_t characterRam[6143];  // $8000-$97FF   Character RAM
	uint8_t hardwareIO[127];     // $8000-$97FF   Hardware IO
	uint8_t zeroPage[127];       // $FF80-$FFFE   Zero Page
	uint8_t interruptEnable;     // $FFFF         Interrupt Enable Register
};

