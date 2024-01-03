#include "AudioDebugger.h"
#include "EventManager.h"

namespace App
{
	AudioDebugger::AudioDebugger(Core::GameBoy* gb) : ImguiWidgetBase("Audio Debugger"), gameboy(gb)
	{
		EventManager::Instance().Subscribe(Event::AUDIO_DEBUGGER_ENABLE, this);
		EventManager::Instance().Subscribe(Event::AUDIO_DEBUGGER_DISABLE, this);
	}

	AudioDebugger::~AudioDebugger() {}

	void AudioDebugger::RenderContent()
	{
		ImGui::SeparatorText("Registers");

		// CPU Info
		ImGui::Text("NR10 CH1 Sweep: $%02x", gameboy->mmu.Read(Core::HW_NR10_SOUND_CHANNEL_1_SWEEP));
		ImGui::Text("NR11 CH1 Length Timer & Duty Cycle: $%02x", gameboy->mmu.Read(Core::HW_NR11_SOUND_CHANNEL_1_LEN_TIMER));
		ImGui::Text("NR12 CH1 Vol & Envelope: $%02x", gameboy->mmu.Read(Core::HW_NR12_SOUND_CHANNEL_1_VOL_ENVELOPE));
		ImGui::Text("NR13 CH1 Period Low: $%02x", gameboy->mmu.Read(Core::HW_NR13_SOUND_CHANNEL_1_PERIOD_LOW));
		ImGui::Text("NR14 CH1 Period High:  $%02x", gameboy->mmu.Read(Core::HW_NR14_SOUND_CHANNEL_1_PERIOD_HIGH));
		ImGui::Text("NR21 CH2 Length Timer & Duty Cycle: $%02x", gameboy->mmu.Read(Core::HW_NR21_SOUND_CHANNEL_2_LEN_TIMER));
		ImGui::Text("NR22 CH2 Vol & Envelope: $%02x", gameboy->mmu.Read(Core::HW_NR22_SOUND_CHANNEL_2_VOL_ENVELOPE));
		ImGui::Text("NR23 CH2 Period Low: $%02x", gameboy->mmu.Read(Core::HW_NR23_SOUND_CHANNEL_2_PERIOD_LOW));
		ImGui::Text("NR24 CH2 Period High: $%02x", gameboy->mmu.Read(Core::HW_NR24_SOUND_CHANNEL_2_PERIOD_HIGH));
		ImGui::Text("NR30 CH3 DAC enable: $%02x", gameboy->mmu.Read(Core::HW_NR30_SOUND_CHANNEL_3_DAC_ENABLE));
		ImGui::Text("NR34 CH3 Length Timer: $%02x", gameboy->mmu.Read(Core::HW_NR31_SOUND_CHANNEL_3_LEN_TIMER));
		ImGui::Text("NR32 CH3 Output Level: $%02x", gameboy->mmu.Read(Core::HW_NR32_SOUND_CHANNEL_3_OUTPUT_LEVEL));
		ImGui::Text("NR33 CH3 Period Low: $%02x", gameboy->mmu.Read(Core::HW_NR33_SOUND_CHANNEL_3_PERIOD_LOW));
		ImGui::Text("NR34 CH3 Period High: $%02x", gameboy->mmu.Read(Core::HW_NR34_SOUND_CHANNEL_3_PERIOD_HIGH));
		ImGui::Text("NR41 CH4 Length Timer: $%02x", gameboy->mmu.Read(Core::HW_NR41_SOUND_CHANNEL_4_LEN_TIMER));
		ImGui::Text("NR42 CH4 Vol & Envelope: $%02x", gameboy->mmu.Read(Core::HW_NR42_SOUND_CHANNEL_4_VOL_ENVELOPE));
		ImGui::Text("NR43 CH4 Freq & Randomness: $%02x", gameboy->mmu.Read(Core::HW_NR43_SOUND_CHANNEL_4_FREQ_RANDOM));
		ImGui::Text("NR44 CH4 Control: $%02x", gameboy->mmu.Read(Core::HW_NR44_SOUND_CHANNEL_4_CONTROL));
		ImGui::Text("NR50 Master Vol & VIN Panning: $%02x", gameboy->mmu.Read(Core::HW_NR50_MASTER_VOLUME));
		ImGui::Text("NR51 Sound Panning: $%02x", gameboy->mmu.Read(Core::HW_NR51_SOUND_PANNING));
		ImGui::Text("NR52 Sound On/Off: $%02x", gameboy->mmu.Read(Core::HW_NR52_SOUND_TOGGLE));
		ImGui::Text("");
		ImGui::SeparatorText("Waveforms");

		// TODO: Need to fix this abomination
		float arr[] =
		{
			static_cast<float>(  gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE) & 0x0F			  ),
			static_cast<float>( (gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE) & 0xF0) >> 4	  ),
			static_cast<float>(  gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 1) & 0x0F		  ),
			static_cast<float>( (gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 1) & 0xF0) >> 4 ),
			static_cast<float>(  gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 2) & 0x0F		  ),
			static_cast<float>( (gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 2) & 0xF0) >> 4 ),
			static_cast<float>(  gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 3) & 0x0F		  ),
			static_cast<float>( (gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 3) & 0xF0) >> 4 ),
			static_cast<float>(  gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 4) & 0x0F		  ),
			static_cast<float>( (gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 4) & 0xF0) >> 4 ),
			static_cast<float>(  gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 5) & 0x0F		  ),
			static_cast<float>( (gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 5) & 0xF0) >> 4 ),
			static_cast<float>(  gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 6) & 0x0F		  ),
			static_cast<float>( (gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 6) & 0xF0) >> 4 ),
			static_cast<float>(  gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 7) & 0x0F		  ),
			static_cast<float>( (gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 7) & 0xF0) >> 4 ),
			static_cast<float>(  gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 8) & 0x0F		  ),
			static_cast<float>( (gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 8) & 0xF0) >> 4 )
		};
		ImGui::PlotLines("WAVE RAM", arr, IM_ARRAYSIZE(arr), 0, "", 0.0f, 15.0f, ImVec2(0.0f, 40.f));

		uint8_t testfull = gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE);
		uint8_t test = gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE) & 0x0F;
		uint8_t test1 = (gameboy->mmu.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE) & 0xF0) >> 4;
	}

	void AudioDebugger::OnEvent(Event event)
	{
		if (event == Event::AUDIO_DEBUGGER_ENABLE)
		{
			ShowWindow = true;
		}
		if (event == Event::AUDIO_DEBUGGER_DISABLE)
		{
			ShowWindow = false;
		}
	}
}