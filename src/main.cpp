#include "GameBoy.h"
#include "Cpu.h"
#include "Cartridge.h"

#include <fstream>
#include <vector>


int main()
{
	GameBoy gb;

	std::shared_ptr<Cartridge> cart;
	cart = std::make_shared<Cartridge>("../hello-world.gb");

	gb.InsertCartridge(*cart);
	gb.Run();


	return true;
}