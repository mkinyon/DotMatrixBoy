#include "Debugger.h"
#include "EventManager.h"

namespace App
{
	Debugger::Debugger(Core::GameBoy* gb) : ImguiWidgetBase("Debugger"), m_GameBoy(gb)
	{
		EventManager::Instance().Subscribe(Event::DEBUGGER_ENABLE, this);
		EventManager::Instance().Subscribe(Event::DEBUGGER_DISABLE, this);

		m_Instructions = m_GameBoy->m_CPU.DisassebleAll();
	}

	Debugger::~Debugger() {}

	void Debugger::RenderContent()
	{
		ImGui::SeparatorText("CPU/PPU State");

		// CPU Info
		ImGui::BeginChild("L", ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, 150), ImGuiChildFlags_None);
		ImGui::Text("Cycles: %d", m_GameBoy->m_CPU.m_TotalCycles);
		ImGui::Text("PC: $%04x", m_GameBoy->m_CPU.State.PC);
		ImGui::Text("SP: $%04x", m_GameBoy->m_CPU.State.SP);
		ImGui::Text("AF: $%04x", m_GameBoy->m_CPU.State.AF);
		ImGui::Text("BC: $%04x", m_GameBoy->m_CPU.State.BC);
		ImGui::Text("DE: $%04x", m_GameBoy->m_CPU.State.DE);
		ImGui::Text("HL: $%04x", m_GameBoy->m_CPU.State.HL);
		ImGui::Text("IE: %01d", m_GameBoy->m_MMU.Read(Core::HW_INTERRUPT_ENABLE));
		ImGui::EndChild();

		ImGui::SameLine();

		// CPU Flags
		ImGui::BeginChild("LM", ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, 150), ImGuiChildFlags_None);
		// Flags
		// todo: the cpu flags are currently read only but should be read/write
		bool z = m_GameBoy->m_CPU.GetCPUFlag(Core::FLAG_ZERO); ImGui::Checkbox("Z", &z);
		bool n = m_GameBoy->m_CPU.GetCPUFlag(Core::FLAG_SUBTRACT); ImGui::Checkbox("N", &n);
		bool h = m_GameBoy->m_CPU.GetCPUFlag(Core::FLAG_HALF_CARRY); ImGui::Checkbox("H", &h);
		bool c = m_GameBoy->m_CPU.GetCPUFlag(Core::FLAG_CARRY); ImGui::Checkbox("C", &c);
		ImGui::EndChild();

		ImGui::SameLine();

		// PPU Info
		ImGui::BeginChild("RM", ImVec2(ImGui::GetContentRegionAvail().x * 0.50f, 150), ImGuiChildFlags_None);
		if (m_GameBoy->m_PPU.GetMode() == Core::MODE_0_HBLANK)  ImGui::Text("Mode: MODE_0_HBLANK");
		if (m_GameBoy->m_PPU.GetMode() == Core::MODE_1_VBLANK)  ImGui::Text("Mode: MODE_1_VBLANK");
		if (m_GameBoy->m_PPU.GetMode() == Core::MODE_2_OAMSCAN) ImGui::Text("Mode: MODE_2_OAMSCAN");
		if (m_GameBoy->m_PPU.GetMode() == Core::MODE_3_DRAWING) ImGui::Text("Mode: MODE_3_DRAWING");
		
		ImGui::Text("LCDC: %1d", m_GameBoy->m_MMU.Read(Core::HW_LCDC_LCD_CONTROL));
		ImGui::Text("STAT: %1d", m_GameBoy->m_MMU.Read(Core::HW_STAT_LCD_STATUS));
		
		ImGui::Text("SCX: %1d", m_GameBoy->m_MMU.Read(Core::HW_SCX_VIEWPORT_X_POS));
		ImGui::Text("SCY: %1d", m_GameBoy->m_MMU.Read(Core::HW_SCY_VIEWPORT_Y_POS));

		ImGui::Text("DIV: %02x", m_GameBoy->m_MMU.Read(Core::HW_DIV_DIVIDER_REGISTER));
		ImGui::Text("TIMA: %02x", m_GameBoy->m_MMU.Read(Core::HW_TIMA_TIMER_COUNTER));
		ImGui::Text("TMA: %02x", m_GameBoy->m_MMU.Read(Core::HW_TMA_TIMER_MODULO));
		ImGui::Text("TAC: %02x", m_GameBoy->m_MMU.Read(Core::HW_TAC_TIMER_CONTROL));
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("R", ImVec2(ImGui::GetContentRegionAvail().x, 150), ImGuiChildFlags_None);
		ImGui::Text("LY (Scanline): %1d", m_GameBoy->m_MMU.Read(Core::HW_LY_LCD_Y_COORD));
		ImGui::Text("LYC: %1d", m_GameBoy->m_MMU.Read(Core::HW_LYC_LY_COMPARE));
		
		ImGui::Text("WX: %1d", m_GameBoy->m_MMU.Read(Core::HW_WX_WINDOW_X_POS));
		ImGui::Text("WY: %1d", m_GameBoy->m_MMU.Read(Core::HW_WY_WINDOW_Y_POS));
		
		ImGui::Text("Dots This Frame: %1d", m_GameBoy->m_PPU.GetTotalDotsThisFrame());
		ImGui::Text("Total Frames: %1d", m_GameBoy->m_PPU.GetTotalFrames());

		ImGui::Text("JOY: %2d", m_GameBoy->m_MMU.Read(Core::HW_P1JOYP_JOYPAD));
		ImGui::EndChild();

		ImGui::SeparatorText("Rom Instructions");

		//// Rom Data
		ImGui::BeginChild("Instruct", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None);

		for (uint16_t i = 0; i < 0x7FFF; i++)
		{
			// todo: currently skipping instructions that are blank.  The cpu should clean this up
			if (!m_Instructions[i].empty())
			{
				bool isCurrentInstr = m_GameBoy->m_CPU.State.PC == i;

				if (isCurrentInstr)
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));

				ImGui::Text(m_Instructions[i].c_str());

				if (isCurrentInstr)
				{
					ImGui::SetScrollHereY();
					ImGui::PopStyleColor();
				}
			}
		}

		ImGui::EndChild();
	}

	void Debugger::OnEvent(Event event)
	{
		if (event == Event::DEBUGGER_ENABLE)
		{
			m_ShowWindow = true;
		}
		if (event == Event::DEBUGGER_DISABLE)
		{
			m_ShowWindow = false;
		}
	}
}