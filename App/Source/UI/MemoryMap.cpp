#include "MemoryMap.h"

namespace App
{
	MemoryMap::MemoryMap(Core::GameBoy& gb) : ImguiWidgetBase("MemoryMap"), gameboy(gb)
	{
	}

	MemoryMap::~MemoryMap() {}


	void MemoryMap::RenderContent()
	{
		uint16_t addr = 0x0000;
		int nColumns = 16;

		for (int row = 0; addr < 0xF000; row++)
		{
			// render address and bytes
			ImGui::Text("$%04x", addr); ImGui::SameLine();
			for (int col = 0; col < nColumns; col++)
			{
				ImGui::Text(" %02x", gameboy.ReadFromMemoryMap(addr)); ImGui::SameLine();
				addr += 1;
			}
			
			// render ascii
			addr -= nColumns;
			ImGui::Text(" "); ImGui::SameLine();
			for (int col = 0; col < nColumns; col++)
			{
				ImGui::Text("%c", gameboy.ReadFromMemoryMap(addr));
				if (col != 15) ImGui::SameLine();
				addr += 1;
			}
		}
	}
}