#pragma once
#include <stdint.h>
#include <vector>

#include "Cpu.h"
#include "Ppu.h"
#include "Cartridge.h"
#include "Utils.h"

class GameBoy
{
public:
	GameBoy();
	~GameBoy();

public:
	void Run(bool enableBootRom);
	void Clock();
	void InsertCartridge(Cartridge &cart);
	uint8_t& ReadFromMemoryMap(uint16_t address);
	void WriteToMemoryMap(uint16_t address, uint8_t value);
	bool ReadFromMemoryMapRegister(uint16_t address, int flag);
	void WriteToMemoryMapRegister(uint16_t address, int flag, bool isEnabled);

public:
	Cpu cpu;
	Ppu ppu;

private:
	//devices
	Cartridge cart;						// $0000-$7FFF   RES and Interrupts, cart header, cart data
	uint8_t characterRam[6143] = {};	// $8000-$97FF   Character RAM
	uint8_t bgMapData1[1023] = {};      // $9800-$9BFF   BG Map Data 1
	uint8_t bgMapData2[1023] = {};      // $9C00-$9FFF   BG Map Data 2
	uint8_t cartRam[8191] = {};			// $A000-$BFFF   Cartridge RAM (If available)
	uint8_t ram[8191] = {};				// $C000-$DFFF   Internal RAM (Bank 0 and Banks 1-7 switchable for CGB)
										// $E000-$FDFF   Echo RAM (Not used)
	uint8_t oam[159] = {};				// $FE00-$FE9F   OAM - Object Attribute Memory
										// $FEA0-$FEFF   Unusable memory region
	uint8_t hardwareIO[127] = {};		// $FF00-$FF7F   Hardware IO
	uint8_t zeroPage[127] = {};			// $FF80-$FFFE   Zero Page
	uint8_t interruptEnable = 0;		// $FFFF         Interrupt Enable Register
};

