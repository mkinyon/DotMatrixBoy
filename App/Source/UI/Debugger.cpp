#include "Debugger.h"
#include "EventManager.h"

namespace App
{
	Debugger::Debugger(Core::GameBoy* gb) : ImguiWidgetBase("Debugger"), m_GameBoy(gb)
	{
		EventManager::Instance().Subscribe(Event::DEBUGGER_ENABLE, this);
		EventManager::Instance().Subscribe(Event::DEBUGGER_DISABLE, this);

		//m_Instructions = m_GameBoy->m_CPU.DisassebleAll();
	}

	Debugger::~Debugger() {}

	void Debugger::RenderContent()
	{
		ImGui::SeparatorText("CPU");
		Core::Cpu::sCPUState* cpuState = m_GameBoy->m_CPU.GetState();

		// CPU Info
		ImGui::Text("Clock Cycles: %d", m_GameBoy->m_CPU.m_TotalCycles);
		ImGui::Separator();

		int flags = ImGuiInputTextFlags_EnterReturnsTrue;

		// only allow editing if we are paused and a game is loaded.
		if (!m_GameBoy->IsPaused() || !m_GameBoy->IsRomLoaded())
		{
			flags |= ImGuiInputTextFlags_ReadOnly;
		}

		ImGui::PushItemWidth(100);
		ImGui::InputScalar("PC", ImGuiDataType_U16, &cpuState->PC, NULL, NULL, "%04X", flags); ImGui::SameLine(0,50);
		ImGui::InputScalar("SP", ImGuiDataType_U16, &cpuState->SP, NULL, NULL, "%04X", flags);
		ImGui::PopItemWidth();
		ImGui::PushItemWidth(50);
		ImGui::InputScalar("AF", ImGuiDataType_U16, &cpuState->AF, NULL, NULL, "%04X", flags); ImGui::SameLine(0, 50);
		ImGui::InputScalar("A", ImGuiDataType_U8, &cpuState->A, NULL, NULL, "%02X", flags); ImGui::SameLine(0, 50);
		ImGui::InputScalar("F", ImGuiDataType_U8, &cpuState->F, NULL, NULL, "%02X", flags);
		ImGui::InputScalar("BC", ImGuiDataType_U16, &cpuState->BC, NULL, NULL, "%04X", flags); ImGui::SameLine(0, 50);
		ImGui::InputScalar("B", ImGuiDataType_U8, &cpuState->B, NULL, NULL, "%02X", flags); ImGui::SameLine(0, 50);
		ImGui::InputScalar("C", ImGuiDataType_U8, &cpuState->C, NULL, NULL, "%02X", flags);
		ImGui::InputScalar("DE", ImGuiDataType_U16, &cpuState->DE, NULL, NULL, "%04X", flags); ImGui::SameLine(0, 50);
		ImGui::InputScalar("D", ImGuiDataType_U8, &cpuState->D, NULL, NULL, "%02X", flags); ImGui::SameLine(0, 50);
		ImGui::InputScalar("E", ImGuiDataType_U8, &cpuState->E, NULL, NULL, "%02X", flags);
		ImGui::InputScalar("HL", ImGuiDataType_U16, &cpuState->HL, NULL, NULL, "%04X", flags); ImGui::SameLine(0, 50);
		ImGui::InputScalar("H", ImGuiDataType_U8, &cpuState->H, NULL, NULL, "%02X", flags); ImGui::SameLine(0, 50);
		ImGui::InputScalar("L", ImGuiDataType_U8, &cpuState->L, NULL, NULL, "%02X", flags);
		ImGui::PopItemWidth();

		bool z = m_GameBoy->m_CPU.GetCPUFlag(Core::FLAG_ZERO);       ImGui::Checkbox("Z", &z); ImGui::SameLine(0,25);
		bool n = m_GameBoy->m_CPU.GetCPUFlag(Core::FLAG_SUBTRACT);   ImGui::Checkbox("N", &n); ImGui::SameLine(0,25);
		bool h = m_GameBoy->m_CPU.GetCPUFlag(Core::FLAG_HALF_CARRY); ImGui::Checkbox("H", &h); ImGui::SameLine(0,25);
		bool c = m_GameBoy->m_CPU.GetCPUFlag(Core::FLAG_CARRY);      ImGui::Checkbox("C", &c);
		ImGui::Text("");


		// Interrupts
		uint8_t ie = m_GameBoy->m_MMU.Read(Core::HW_FFFF_INTERRUPT_ENABLE);
		bool ie_vb = ie & Core::IE_Flags::IE_VBLANK; ImGui::Checkbox("VBLANK (IE)", &ie_vb);	       ImGui::SameLine(0, 25);
		bool ie_lcd = ie & Core::IE_Flags::IE_LCD; ImGui::Checkbox("LCD (IE)", &ie_lcd);		       ImGui::SameLine(0, 25);
		bool ie_timer = ie & Core::IE_Flags::IE_TIMER; ImGui::Checkbox("TIMER (IE)", &ie_timer);       ImGui::SameLine(0, 25);
		bool ie_serial = ie & Core::IE_Flags::IE_SERIAL; ImGui::Checkbox("SERIAL (IE)", &ie_serial);   ImGui::SameLine(0, 25);
		bool ie_jp = ie & Core::IE_Flags::IE_JOYPAD; ImGui::Checkbox("JOYPAD (IE)", &ie_jp);

		uint8_t intf = m_GameBoy->m_MMU.Read(Core::HW_FF0F_IF_INTERRUPT_FLAG);
		bool if_vb = intf & Core::IF_Flags::IF_VBLANK; ImGui::Checkbox("VBLANK (IF)", &if_vb);	       ImGui::SameLine(0, 25);
		bool if_lcd = intf & Core::IF_Flags::IF_LCD; ImGui::Checkbox("LCD (IF)", &if_lcd);			   ImGui::SameLine(0, 25);
		bool if_timer = intf & Core::IF_Flags::IF_TIMER; ImGui::Checkbox("TIMER (IF)", &if_timer);     ImGui::SameLine(0, 25);
		bool if_serial = intf & Core::IF_Flags::IF_SERIAL; ImGui::Checkbox("SERIAL (IF)", &if_serial); ImGui::SameLine(0, 25);
		bool if_jp = intf & Core::IF_Flags::IF_JOYPAD; ImGui::Checkbox("JOYPAD (IF)", &if_jp);
		ImGui::Text("");

		// Timer
		ImGui::SeparatorText("Timer");
		ImGui::Text("Internal DIV: %02x", m_GameBoy->m_MMU.Read(Core::HW_FF03_DIV_DIVIDER_REGISTER_LOW));
		ImGui::Text("DIV: %02x", m_GameBoy->m_MMU.Read(Core::HW_FF04_DIV_DIVIDER_REGISTER));
		ImGui::Text("TIMA: %02x", m_GameBoy->m_MMU.Read(Core::HW_FF05_TIMA_TIMER_COUNTER));
		ImGui::Text("TMA: %02x", m_GameBoy->m_MMU.Read(Core::HW_FF06_TMA_TIMER_MODULO));

		bool tacEnabled = m_GameBoy->m_MMU.ReadRegisterBit(Core::HW_FF07_TAC_TIMER_CONTROL, Core::TAC_ENABLE);
		ImGui::Text("TAC Enabled: %s", tacEnabled ? "True" : "False");

		uint8_t selectedClock = m_GameBoy->m_MMU.Read(Core::HW_FF07_TAC_TIMER_CONTROL) & 0x3;
		ImGui::Text("TAC Clock Select: %02x", selectedClock);
		ImGui::Text("");

		// PPU Info
		ImGui::SeparatorText("PPU");

		ImGui::BeginChild("RM", ImVec2(ImGui::GetContentRegionAvail().x * 0.50f, 160), ImGuiChildFlags_None);
		if (m_GameBoy->m_PPU.GetMode() == Core::MODE_0_HBLANK)  ImGui::Text("Mode: MODE_0_HBLANK");
		if (m_GameBoy->m_PPU.GetMode() == Core::MODE_1_VBLANK)  ImGui::Text("Mode: MODE_1_VBLANK");
		if (m_GameBoy->m_PPU.GetMode() == Core::MODE_2_OAMSCAN) ImGui::Text("Mode: MODE_2_OAMSCAN");
		if (m_GameBoy->m_PPU.GetMode() == Core::MODE_3_DRAWING) ImGui::Text("Mode: MODE_3_DRAWING");
		
		ImGui::Text("LCDC: %1d", m_GameBoy->m_MMU.Read(Core::HW_FF40_LCDC_LCD_CONTROL));
		ImGui::Text("STAT: %1d", m_GameBoy->m_MMU.Read(Core::HW_FF41_STAT_LCD_STATUS));
		
		ImGui::Text("SCX: %1d", m_GameBoy->m_MMU.Read(Core::HW_FF43_SCX_VIEWPORT_X_POS));
		ImGui::Text("SCY: %1d", m_GameBoy->m_MMU.Read(Core::HW_FF42_SCY_VIEWPORT_Y_POS));

		
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("R", ImVec2(ImGui::GetContentRegionAvail().x, 150), ImGuiChildFlags_None);
		ImGui::Text("LY (Scanline): %1d", m_GameBoy->m_MMU.Read(Core::HW_FF44_LY_LCD_Y_COORD));
		ImGui::Text("LYC: %1d", m_GameBoy->m_MMU.Read(Core::HW_FF45_LYC_LY_COMPARE));
		
		ImGui::Text("WX: %1d", m_GameBoy->m_MMU.Read(Core::HW_FF4B_WX_WINDOW_X_POS));
		ImGui::Text("WY: %1d", m_GameBoy->m_MMU.Read(Core::HW_FF4A_WY_WINDOW_Y_POS));
		
		ImGui::Text("Dots This Frame: %1d", m_GameBoy->m_PPU.GetTotalDotsThisFrame());
		ImGui::Text("Total Frames: %1d", m_GameBoy->m_PPU.GetTotalFrames());

		ImGui::Text("JOY: %2d", m_GameBoy->m_MMU.Read(Core::HW_FF00_P1JOYP_JOYPAD));
		ImGui::EndChild();


		// ROM instructions
		ImGui::SeparatorText("Rom Instructions");
		ImGui::BeginChild("Instruct", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None);

		for (uint16_t i = 0; i < 0x7FFF; i++)
		{
			// todo: currently skipping instructions that are blank.  The cpu should clean this up
			if (!m_Instructions[i].empty())
			{
				bool isCurrentInstr = m_GameBoy->m_CPU.GetState()->PC == i;

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