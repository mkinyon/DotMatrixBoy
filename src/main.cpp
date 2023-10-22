#include "cpu.h"
#include "cartridge.h"

#include <fstream>
#include <vector>


int main()
{
	std::shared_ptr<Cartridge> cart;
	std::shared_ptr<Cpu> cpu;

	cart = std::make_shared<Cartridge>("../tetris.gb");
	cpu = std::make_shared<Cpu>();

	int pc = 0x100; // game boy execution start point

	// load cart data into CPU
	cpu->loadCartData(&cart->romData);

	// this is not ideal but at least allows me run the cpu until 
	// an unimplemented cpu instruction is found.
	while (true)
	{
		cpu->clock();
	}

	// disassemble
	while (pc < cart->romData.size())
	{
		pc += cpu->disassemble(&cart->romData, pc);
	}

	// Called once at the start, so create things here
	return true;
}