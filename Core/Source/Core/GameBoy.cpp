#include "GameBoy.h"
#include "Logger.h"

namespace Core
{
	GameBoy::GameBoy(Cartridge& cart) : m_Cart(cart), m_MMU(cart), m_CPU(m_MMU), m_PPU(m_MMU), m_APU(m_MMU), m_Input(m_MMU) {}
	GameBoy::~GameBoy() {}

	void GameBoy::Run(bool enableBootRom)
	{
		m_CPU.Reset(enableBootRom);
	}

	// this is deprecated since we are using the audio callback 
	// to clock the gameboy
	void GameBoy::Clock(float elapsedTimeMS)
	{
		if (!m_IsPaused)
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

	void GameBoy::FeedAudioBuffer(uint8_t* stream, int len)
	{
		if (!m_IsPaused)
		{
			// 97280 = 95 cycles per sample * 1024 samples to fill
			for (int i = 0; i < 97280; i++)
			{
				ClockSystems();
			}
		}

		m_APU.FeedAudioBuffer(stream, len);
	}

	void GameBoy::ClockSystems()
	{
		m_CPU.Clock();
		m_PPU.Clock();

		m_APU.Clock();
		m_Input.Clock();
	}
}