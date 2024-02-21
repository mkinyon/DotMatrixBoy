#pragma once
#include <stdint.h>
#include <vector>

#include "Mmu.h"
#include "Cpu.h"
#include "Ppu.h"
#include "Audio/Apu.h"
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
		void FeedAudioBuffer(uint8_t* stream, int len);

	public:
		Cartridge& m_Cart;
		Mmu m_MMU;
		Cpu m_CPU;
		Ppu m_PPU;
		Apu m_APU;
		Input m_Input;

	private:
		bool m_IsPaused = false;
	};
}

