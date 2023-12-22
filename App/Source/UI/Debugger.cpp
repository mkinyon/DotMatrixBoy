#include "Debugger.h"

namespace App
{
	Debugger::Debugger(Core::GameBoy& gb) : ImguiWindowBase("CPU Debugger"), gameboy(gb)
	{
		instructions = gameboy.cpu.DisassebleAll();
	}

	Debugger::~Debugger() {}


	void Debugger::RenderContent()
	{
		ImGui::SeparatorText("CPU State");

		// CPU Info
		ImGui::BeginChild("R", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 120), ImGuiChildFlags_None);
		ImGui::Text("PC: $%04x", gameboy.cpu.State.PC);
		ImGui::Text("SP: $%04x", gameboy.cpu.State.SP);
		ImGui::Text("AF: $%04x", gameboy.cpu.State.AF);
		ImGui::Text("BC: $%04x", gameboy.cpu.State.BC);
		ImGui::Text("DE: $%04x", gameboy.cpu.State.DE);
		ImGui::Text("HL: $%04x", gameboy.cpu.State.HL);
		ImGui::Text("IE: %01d", gameboy.ReadFromMemoryMap(Core::HW_INTERRUPT_ENABLE));
		ImGui::EndChild();

		ImGui::SameLine();

		// CPU Flags
		ImGui::BeginChild("L", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 120), ImGuiChildFlags_None);
		// Flags
		// todo: the cpu flags are currently read only but should be read/write
		bool z = gameboy.cpu.GetCPUFlag(Core::FLAG_ZERO); ImGui::Checkbox("Z", &z);
		bool n = gameboy.cpu.GetCPUFlag(Core::FLAG_SUBTRACT); ImGui::Checkbox("N", &n);
		bool h = gameboy.cpu.GetCPUFlag(Core::FLAG_HALF_CARRY); ImGui::Checkbox("H", &h);
		bool c = gameboy.cpu.GetCPUFlag(Core::FLAG_CARRY); ImGui::Checkbox("C", &c);
		ImGui::EndChild();


		ImGui::SeparatorText("Rom Instructions");

		// Rom Data
		ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x, 260), ImGuiChildFlags_None);

		for (uint16_t i = 0; i < 0x7FFF; i++)
		{
			// todo: currently skipping instructions that are blank.  The cpu should clean this up
			if (!instructions[i].empty())
			{
				bool isCurrentInstr = gameboy.cpu.State.PC == i;

				if (isCurrentInstr)
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));

				ImGui::Text(instructions[i].c_str());

				if (isCurrentInstr)
				{
					ImGui::SetScrollHereY();
					ImGui::PopStyleColor();
				}
			}
		}

		ImGui::EndChild();
	}
}