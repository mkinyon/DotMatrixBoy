#include "GameBoy.h"


GameBoy::GameBoy() : cpu() {}

GameBoy::~GameBoy() {}

void GameBoy::Run()
{
	cpu.Reset(*this);
	int pc = 0x100; // game boy execution start point

	//while (true)
	//{
	//	cpu.Clock(*this);
	//}

	//// disassemble all instructions (for testing)
	//while (pc < cart.romData->size())
	//{
	//	pc += cpu.Disassemble(&ReadFromMemoryMap(pc), pc);
	//}
}

void GameBoy::InsertCartridge(Cartridge &cartridge)
{
	cart = cartridge;
}

uint8_t& GameBoy::ReadFromMemoryMap(uint16_t address)
{
	// cartridge rom
	if (address >= 0x0000 && address <= 0x7FFF)
	{
		return cart.Read(address);
	}
	// internal ram bank 0
	else if (address >= 0xC000 && address <= 0xCFFF)
	{
		uint8_t offset = address - 0xC000;
		return ram[offset];
	}
	// bg map data 2
	else if (address >= 0x9C00 && address <= 0x9FFF)
	{
		uint8_t offset = address - 0x9C00;
		return bgMapData2[offset];
	}
	// bg map data 1
	else if (address >= 0x9800 && address <= 0x9BFF)
	{
		uint8_t offset = address - 0x9800;
		return bgMapData1[offset];
	}
	// character ram
	else if (address >= 0x8000 && address <= 0x97FF)
	{
		uint8_t offset = address - 0x8000;
		return characterRam[offset];
	}
	// hardware IO
	else if (address >= 0xFF00 && address <= 0xFF7F)
	{
		uint8_t offset = address - 0xFF00;
		return hardwareIO[offset];
	}
	// zero page
	else if (address >= 0xFF80 && address <= 0xFFFE)
	{
		uint8_t offset = address - 0xFF80;
		return zeroPage[offset];
	}
	// interrupt enable register
	else if (address == 0xFFFF)
	{
		return interruptEnable;
	}
	else
	{
		int test = 0;
	}
}

void GameBoy::WriteToMemoryMap(uint16_t address, uint8_t value)
{
	// internal ram
	if (address >= 0xC000 && address <= 0xCFFF)
	{
		uint8_t offset = address - 0xC000;
		ram[offset] = value;
	}
	// bg map data 2
	else if (address >= 0x9C00 && address <= 0x9FFF)
	{
		uint8_t offset = address - 0x9C00;
		bgMapData2[offset] = value;
	}
	// bg map data 1
	else if (address >= 0x9800 && address <= 0x9BFF)
	{
		uint8_t offset = address - 0x9800;
		bgMapData1[offset] = value;
	}
	// character ram
	else if (address >= 0x8000 && address <= 0x97FF)
	{
		uint8_t offset = address - 0x8000;
		characterRam[offset] = value;
	}
	// hardware IO
	else if (address >= 0xFF00 && address <= 0xFF7F)
	{
		uint8_t offset = address - 0xFF00;
		hardwareIO[offset] = value;
	}
	// zero page
	else if (address >= 0xFF80 && address <= 0xFFFE)
	{
		uint8_t offset = address - 0xFF80;
		zeroPage[offset] = value;
	}
	// interrupt enable register
	else if (address == 0xFFFF)
	{
		interruptEnable = value;
	}
	else
	{
		int test = 0;
	}
}
