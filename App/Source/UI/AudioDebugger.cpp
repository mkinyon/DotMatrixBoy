#include "AudioDebugger.h"
#include "EventManager.h"
#include "Core\Defines.h"
#include <vector>

namespace App
{
	AudioDebugger::AudioDebugger(Core::GameBoy* gb) : ImguiWidgetBase("Audio Debugger"), m_GameBoy(gb)
	{
		EventManager::Instance().Subscribe(Event::AUDIO_DEBUGGER_ENABLE, this);
		EventManager::Instance().Subscribe(Event::AUDIO_DEBUGGER_DISABLE, this);
	}

	AudioDebugger::~AudioDebugger() {}

	void AudioDebugger::RenderContent()
	{
		ImGui::Text("Ring Buffer: %d", m_GameBoy->m_APU.GetRingBufferSize());

		ImGui::SeparatorText("Master Mix");
		bool masterEnabled = m_GameBoy->m_MMU.ReadRegisterBit(Core::HW_NR52_SOUND_TOGGLE, Core::NR52_AUDIO_ON); ImGui::Checkbox("Enabled", &masterEnabled);

		std::vector<float> masterBuffer = m_GameBoy->m_APU.GetMasterAudioBuffer();
		ImGui::PlotLines("Master", masterBuffer.data(), masterBuffer.size(), 0, nullptr, -0.5f, 0.5f, ImVec2(0, 40.0f));


		ImGui::SeparatorText("Channel 1 - Square");
		bool ch1Enabled = m_GameBoy->m_MMU.ReadRegisterBit(Core::HW_NR52_SOUND_TOGGLE, Core::NR52_CH1_ON); ImGui::Checkbox("Enabled", &ch1Enabled);

		std::vector<float> ch1Buffer = m_GameBoy->m_APU.GetCh1AudioBuffer();
		ImGui::PlotLines("CH1 - Square", ch1Buffer.data(), ch1Buffer.size(), 0, nullptr, -1.0f, 1.0f, ImVec2(0, 40.0f));


		ImGui::SeparatorText("Channel 2 - Square");
		bool ch2Enabled = m_GameBoy->m_MMU.ReadRegisterBit(Core::HW_NR52_SOUND_TOGGLE, Core::NR52_CH2_ON); ImGui::Checkbox("Enabled", &ch2Enabled);

		std::vector<float> ch2Buffer = m_GameBoy->m_APU.GetCh2AudioBuffer();
		ImGui::PlotLines("CH2 - Square", ch2Buffer.data(), ch2Buffer.size(), 0, nullptr, -1.0f, 1.0f, ImVec2(0, 40.0f));


		ImGui::SeparatorText("Channel 3 - Wave");
		bool ch3Enabled = m_GameBoy->m_MMU.ReadRegisterBit(Core::HW_NR52_SOUND_TOGGLE, Core::NR52_CH3_ON); ImGui::Checkbox("Enabled", &ch3Enabled);

		std::vector<float> ch3Buffer = m_GameBoy->m_APU.GetCh3AudioBuffer();
		ImGui::PlotLines("CH3 - Wave", ch3Buffer.data(), ch3Buffer.size(), 0, nullptr, -1.0f, 1.0f, ImVec2(0, 40.0f));

		// TODO: Need to fix this abomination
		float arr[] =
		{
			static_cast<float>(m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE) & 0x0F),
			static_cast<float>((m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE) & 0xF0) >> 4),
			static_cast<float>(m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 1) & 0x0F),
			static_cast<float>((m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 1) & 0xF0) >> 4),
			static_cast<float>(m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 2) & 0x0F),
			static_cast<float>((m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 2) & 0xF0) >> 4),
			static_cast<float>(m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 3) & 0x0F),
			static_cast<float>((m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 3) & 0xF0) >> 4),
			static_cast<float>(m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 4) & 0x0F),
			static_cast<float>((m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 4) & 0xF0) >> 4),
			static_cast<float>(m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 5) & 0x0F),
			static_cast<float>((m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 5) & 0xF0) >> 4),
			static_cast<float>(m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 6) & 0x0F),
			static_cast<float>((m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 6) & 0xF0) >> 4),
			static_cast<float>(m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 7) & 0x0F),
			static_cast<float>((m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 7) & 0xF0) >> 4),
			static_cast<float>(m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 8) & 0x0F),
			static_cast<float>((m_GameBoy->m_MMU.Read(Core::HW_WAVRAM_WAVEFORM_STORAGE + 8) & 0xF0) >> 4)
		};
		ImGui::PlotLines("WAVE RAM", arr, IM_ARRAYSIZE(arr), 0, "", 0.0f, 15.0f, ImVec2(0.0f, 40.f));


		ImGui::SeparatorText("Channel 4 - Noise");
		bool ch4Enabled = m_GameBoy->m_MMU.ReadRegisterBit(Core::HW_NR52_SOUND_TOGGLE, Core::NR52_CH4_ON); ImGui::Checkbox("Enabled", &ch4Enabled);

		std::vector<float> ch4Buffer = m_GameBoy->m_APU.GetCh4AudioBuffer();
		ImGui::PlotLines("CH4 - Noise", ch4Buffer.data(), ch4Buffer.size(), 0, nullptr, -1.0f, 1.0f, ImVec2(0, 40.0f));

		ImGui::SeparatorText("Registers");

		// CPU Info
		ImGui::Text("NR10 CH1 Sweep: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR10_SOUND_CHANNEL_1_SWEEP));
		ImGui::Text("NR11 CH1 Length Timer & Duty Cycle: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR11_SOUND_CHANNEL_1_LEN_TIMER));
		ImGui::Text("NR12 CH1 Vol & Envelope: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR12_SOUND_CHANNEL_1_VOL_ENVELOPE));
		ImGui::Text("NR13 CH1 Period Low: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR13_SOUND_CHANNEL_1_PERIOD_LOW));
		ImGui::Text("NR14 CH1 Period High:  $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR14_SOUND_CHANNEL_1_PERIOD_HIGH));
		ImGui::Text("NR21 CH2 Length Timer & Duty Cycle: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR21_SOUND_CHANNEL_2_LEN_TIMER));
		ImGui::Text("NR22 CH2 Vol & Envelope: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR22_SOUND_CHANNEL_2_VOL_ENVELOPE));
		ImGui::Text("NR23 CH2 Period Low: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR23_SOUND_CHANNEL_2_PERIOD_LOW));
		ImGui::Text("NR24 CH2 Period High: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR24_SOUND_CHANNEL_2_PERIOD_HIGH));
		ImGui::Text("NR30 CH3 DAC enable: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR30_SOUND_CHANNEL_3_DAC_ENABLE));
		ImGui::Text("NR34 CH3 Length Timer: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR31_SOUND_CHANNEL_3_LEN_TIMER));
		ImGui::Text("NR32 CH3 Output Level: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR32_SOUND_CHANNEL_3_OUTPUT_LEVEL));
		ImGui::Text("NR33 CH3 Period Low: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR33_SOUND_CHANNEL_3_PERIOD_LOW));
		ImGui::Text("NR34 CH3 Period High: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR34_SOUND_CHANNEL_3_PERIOD_HIGH));
		ImGui::Text("NR41 CH4 Length Timer: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR41_SOUND_CHANNEL_4_LEN_TIMER));
		ImGui::Text("NR42 CH4 Vol & Envelope: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR42_SOUND_CHANNEL_4_VOL_ENVELOPE));
		ImGui::Text("NR43 CH4 Freq & Randomness: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR43_SOUND_CHANNEL_4_FREQ_RANDOM));
		ImGui::Text("NR44 CH4 Control: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR44_SOUND_CHANNEL_4_CONTROL));
		ImGui::Text("NR50 Master Vol & VIN Panning: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR50_MASTER_VOLUME));
		ImGui::Text("NR51 Sound Panning: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR51_SOUND_PANNING));
		ImGui::Text("NR52 Sound On/Off: $%02x", m_GameBoy->m_MMU.Read(Core::HW_NR52_SOUND_TOGGLE));
		ImGui::Text("");
	
	}

	void AudioDebugger::OnEvent(Event event)
	{
		if (event == Event::AUDIO_DEBUGGER_ENABLE)
		{
			m_ShowWindow = true;
		}
		if (event == Event::AUDIO_DEBUGGER_DISABLE)
		{
			m_ShowWindow = false;
		}
	}
}