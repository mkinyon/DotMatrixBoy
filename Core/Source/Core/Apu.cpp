#include "GameBoy.h"
#include "Apu.h"

namespace Core
{
	Apu::Apu(GameBoy& gb) : gameboy(gb)
	{

	}

	Apu::~Apu() {}

	void Apu::Clock()
	{

	}
}