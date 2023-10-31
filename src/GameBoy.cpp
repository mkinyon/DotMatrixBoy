#include "GameBoy.h"


GameBoy::GameBoy() {}

GameBoy::~GameBoy() {}

void GameBoy::Run()
{
	cpu.Reset(*this);
	int pc = 0x100; // game boy execution start point

	while (true)
	{
		cpu.Clock(*this);
	}

	// disassemble all instructions (for testing)
	while (pc < cart.romData->size())
	{
		pc += cpu.Disassemble(&ReadFromMemoryMap(pc), pc);
	}
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
	// zero page
	else if (address >= 0xFF80 && address <= 0xFFFE)
	{
		uint8_t offset = address - 0xFF80;
		return zeroPage[offset];
	}
	else
	{
		// todo: temp
		return cart.Read(0x0000);
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
	// zero page
	else if (address >= 0xFF80 && address <= 0xFFFE)
	{
		uint8_t offset = address - 0xFF80;
		zeroPage[offset] = value;
	}
}
