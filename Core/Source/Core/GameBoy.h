#pragma once
#include <stdint.h>
#include <vector>

#include "Mmu.h"
#include "Cpu.h"
#include "Ppu.h"
#include "Apu.h"
#include "Input.h"
#include "Cartridge.h"
#include "Utils.h"

namespace Core
{
	class GameBoy
	{
	public:
		GameBoy(Cartridge& cart);
		~GameBoy();

	public:
		void Run(bool enableBootRom);
		void Clock(float elapsedTimeMS);
		void Pause();
		void Unpause();
		bool IsPaused();
		void StepCPU();

	public:
		Cartridge& cart;
		Mmu mmu;
		Cpu cpu;
		Ppu ppu;
		Apu apu;
		Input input;

	private:
		bool m_isPaused = false;
	};
}

