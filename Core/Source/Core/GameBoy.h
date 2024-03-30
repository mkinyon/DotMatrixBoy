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
		GameBoy(bool enableBootRom);
		~GameBoy();

	public:
		void LoadRom(const char* file);
		bool IsRomLoaded();

		void Run();
		void Clock(float elapsedTimeMS);

		void Pause();
		void Unpause();
		bool IsPaused();

		void StepCPU();
		void AdvanceFrame();

		bool IsBootRomEnabled();
		void SetBootRomEnabled(bool enabled);

		Cartridge* GetCart();

	private:
		void ClockSystems();
		void Reset();

	public:
		Cartridge* m_Cart = nullptr;
		Mmu m_MMU;
		Cpu m_CPU;
		Ppu m_PPU;
		Apu m_APU;
		Input m_Input;

	private:
		bool m_IsPaused = true;
		bool m_BootRomEnabled = false;
	};
}

