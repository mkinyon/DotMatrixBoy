#include "GameBoy.h"
#include "Logger.h"

namespace Core
{
	GameBoy::GameBoy(bool enableBootRom) : m_MMU(), m_CPU(m_MMU), m_PPU(m_MMU), m_APU(m_MMU), m_Input(m_MMU)
	{
		m_BootRomEnabled = enableBootRom;
	}

	GameBoy::~GameBoy() {}

	void GameBoy::LoadRom(const char* file)
	{
		// pause the emulator so we don't risk causing
		// issues while the cart is replaced.
		Pause();

		delete m_Cart;
		m_Cart = new Cartridge(file, m_BootRomEnabled);
		m_MMU.SetCart(m_Cart);

		Run();
	}

	void GameBoy::Run()
	{
		m_CPU.Reset(m_BootRomEnabled);
		m_PPU.Reset();
		Unpause();
	}

	void GameBoy::Clock(float elapsedTimeMS)
	{
		if (!m_IsPaused && m_Cart != nullptr)
		{
			float cyclesToRun = elapsedTimeMS * Core::CYCLES_PER_MS;

			for (int i = 0; i < cyclesToRun; i++)
			{
				ClockSystems();
			}
		}
	}

	void GameBoy::Pause()
	{
		m_IsPaused = true;
	}

	void GameBoy::Unpause()
	{
		m_IsPaused = false;
	}

	bool GameBoy::IsPaused()
	{
		return m_IsPaused;
	}

	void GameBoy::StepCPU()
	{
		do
		{
			ClockSystems();
		} 
		while (!m_CPU.m_InstructionCompleted);
	}

	void GameBoy::AdvanceFrame()
	{
		for (int i = 0; i < 70368; i++)
		{
			ClockSystems();
		}
	}

	void GameBoy::ClockSystems()
	{
		m_CPU.Clock();
		m_PPU.Clock();

		m_APU.Clock();
		m_Input.Clock();
	}

	bool GameBoy::IsBootRomEnabled()
	{
		return m_BootRomEnabled;
	}

	void GameBoy::SetBootRomEnabled(bool enabled)
	{
		m_BootRomEnabled = enabled;
	}

	Cartridge* GameBoy::GetCart()
	{
		return m_Cart;
	}

}