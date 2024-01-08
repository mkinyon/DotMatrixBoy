#include "GameBoy.h"
#include "Logger.h"

namespace Core
{
	GameBoy::GameBoy(Cartridge& cart) : cart(cart), mmu(cart), cpu(mmu), ppu(mmu), apu(mmu), input(mmu) {}
	GameBoy::~GameBoy() {}

	void GameBoy::Run(bool enableBootRom)
	{
		cpu.Reset(enableBootRom);
	}

	void GameBoy::Clock(float elapsedTimeMS)
	{
		if (!m_isPaused)
		{
			float cyclesToRun = elapsedTimeMS * Core::CYCLES_PER_MS;

			for (int i = 0; i < cyclesToRun; i++)
			{
				cpu.Clock();

				for (int i = 0; i < 4; i++)
				{
					ppu.Clock();
				}

				apu.Clock();
				input.Clock();
			}
		}
	}

	void GameBoy::Pause()
	{
		m_isPaused = true;
	}

	void GameBoy::Unpause()
	{
		m_isPaused = false;
	}

	bool GameBoy::IsPaused()
	{
		return m_isPaused;
	}

	void GameBoy::StepCPU()
	{
		do
		{
			cpu.Clock();

			for (int i = 0; i < 4; i++)
			{
				ppu.Clock();
			}
		} while (!cpu.m_InstructionCompleted);
	}

	void GameBoy::FeedAudioBuffer(uint8_t* stream, int len)
	{
		apu.FeedAudioBuffer(stream, len);
	}
}