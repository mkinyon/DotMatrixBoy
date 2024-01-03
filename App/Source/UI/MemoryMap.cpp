#include "MemoryMap.h"
#include "EventManager.h"

namespace App
{
	MemoryMap::MemoryMap(Core::GameBoy* gb) : ImguiWidgetBase("Memory Map"), gameboy(gb)
	{
		EventManager::Instance().Subscribe(Event::MEMORY_MAP_ENABLE, this);
		EventManager::Instance().Subscribe(Event::MEMORY_MAP_DISABLE, this);
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
				ImGui::Text(" %02x", gameboy->mmu.Read(addr)); ImGui::SameLine();
				addr += 1;
			}
			
			// render ascii
			addr -= nColumns;
			ImGui::Text(" "); ImGui::SameLine();
			for (int col = 0; col < nColumns; col++)
			{
				ImGui::Text("%c", gameboy->mmu.Read(addr));
				if (col != 15) ImGui::SameLine();
				addr += 1;
			}
		}
	}

	void MemoryMap::OnEvent(Event event)
	{
		if (event == Event::MEMORY_MAP_ENABLE)
		{
			ShowWindow = true;
		}
		if (event == Event::MEMORY_MAP_DISABLE)
		{
			ShowWindow = false;
		}
	}
}