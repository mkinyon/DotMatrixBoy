#include "MemoryMap.h"
#include "EventManager.h"

#include <imgui_memory_editor.h>

namespace App
{
	MemoryMap::MemoryMap(Core::GameBoy* gb) : ImguiWidgetBase("Memory Map"), m_GameBoy(gb)
	{
		m_DisableTitleWrapper = true;

		EventManager::Instance().Subscribe(Event::MEMORY_MAP_ENABLE, this);
		EventManager::Instance().Subscribe(Event::MEMORY_MAP_DISABLE, this);
	}

	MemoryMap::~MemoryMap() {}

	void MemoryMap::RenderContent()
	{
		static MemoryEditor memEditor;

		memEditor.DrawWindow("Memory Editor (MMU)", &m_GameBoy->m_MMU.GetMemory()->front(), m_GameBoy->m_MMU.GetMemory()->size());
		if (m_GameBoy->IsRomLoaded())
		{
			memEditor.DrawWindow("Memory Editor (Cart)", &m_GameBoy->GetCart()->GetRomData()->front(), m_GameBoy->GetCart()->GetRomData()->size());
		}
	}

	void MemoryMap::OnEvent(Event event)
	{
		if (event == Event::MEMORY_MAP_ENABLE)
		{
			m_ShowWindow = true;
		}
		if (event == Event::MEMORY_MAP_DISABLE)
		{
			m_ShowWindow = false;
		}
	}
}