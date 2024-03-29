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
		Reset();
		m_CPU.Reset(m_BootRomEnabled);
		m_PPU.Reset();
		Unpause();
	}

	void GameBoy::Reset()
	{
		// randomize memory
		for (uint16_t address = 0x8000; address <= 0x97FF; address++)
		{
			m_MMU.Write(address, GenerateRandomNumber(0, 255));
		}

		// reset div
		m_MMU.Write(HW_FF04_DIV_DIVIDER_REGISTER, 0xAC);
		m_MMU.Write(HW_FF03_DIV_DIVIDER_REGISTER_LOW, 0x00);

		// reset audio
		m_MMU.Write(HW_FF26_NR52_SOUND_TOGGLE, 0xF1);
		m_MMU.Write(HW_FF11_NR11_SOUND_CH1_LEN_TIMER, 0x80);
		m_MMU.Write(HW_FF12_NR12_SOUND_CH1_VOL_ENVELOPE, 0xF3);
		m_MMU.Write(HW_FF14_NR14_SOUND_CH1_PERIOD_HIGH, 0x80);
		m_MMU.Write(HW_FF24_NR50_MASTER_VOLUME, 0x77);
		m_MMU.Write(HW_FF25_NR51_SOUND_PANNING, 0xF3);

		// reset wave
		m_MMU.Write(HW_FF30_WAVRAM_WAVEFORM_STORAGE_0, 0x60);
		m_MMU.Write(HW_FF31_WAVRAM_WAVEFORM_STORAGE_1, 0x0D);
		m_MMU.Write(HW_FF32_WAVRAM_WAVEFORM_STORAGE_2, 0xDA);
		m_MMU.Write(HW_FF33_WAVRAM_WAVEFORM_STORAGE_3, 0xDD);
		m_MMU.Write(HW_FF34_WAVRAM_WAVEFORM_STORAGE_4, 0x50);
		m_MMU.Write(HW_FF35_WAVRAM_WAVEFORM_STORAGE_5, 0x0F);
		m_MMU.Write(HW_FF36_WAVRAM_WAVEFORM_STORAGE_6, 0xAD);
		m_MMU.Write(HW_FF37_WAVRAM_WAVEFORM_STORAGE_7, 0xED);
		m_MMU.Write(HW_FF38_WAVRAM_WAVEFORM_STORAGE_8, 0xC0);
		m_MMU.Write(HW_FF39_WAVRAM_WAVEFORM_STORAGE_9, 0xDE);
		m_MMU.Write(HW_FF3A_WAVRAM_WAVEFORM_STORAGE_A, 0xF0);
		m_MMU.Write(HW_FF3B_WAVRAM_WAVEFORM_STORAGE_B, 0x0D);
		m_MMU.Write(HW_FF3C_WAVRAM_WAVEFORM_STORAGE_C, 0xBE);
		m_MMU.Write(HW_FF3D_WAVRAM_WAVEFORM_STORAGE_D, 0xEF);
		m_MMU.Write(HW_FF3E_WAVRAM_WAVEFORM_STORAGE_E, 0xEF);
		m_MMU.Write(HW_FF3F_WAVRAM_WAVEFORM_STORAGE_F, 0xED);
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