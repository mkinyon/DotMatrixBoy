#include "GameBoy.h"


GameBoy::GameBoy()
{
	memoryMap[65535];
}

GameBoy::~GameBoy() {}

void GameBoy::Run()
{
	int pc = 0x100; // game boy execution start point

	while (true)
	{
		cpu.clock(*this);
	}

	// disassemble all instructions (for testing)
	while (pc < cart.romData->size())
	{
		pc += cpu.disassemble(&ReadFromMemoryMap(pc), pc);
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
		return cart.read(address);
	}
	// internal ram bank 0
	else if (address >= 0xC000 && address <= 0xCFFF)
	{
		//return (uint8_t)0; // todo
	}
}

void GameBoy::WriteToMemoryMap(uint16_t address, uint8_t value)
{
	if (address < 0x2000)
	{
		return;
	}
	if (address >= 0x4000)
	{
		return;
	}

	// TODO state.memory->at(address) = value;
}
