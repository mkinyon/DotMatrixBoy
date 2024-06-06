#pragma once

#include "ImguiWidgetBase.h"
#include "EventObserver.h"
#include "Core/GameBoy.h"
#include "Core/Defines.h"

namespace App
{
	class MemoryMap : public ImguiWidgetBase, public EventObserver
	{
	public:
		MemoryMap(Core::GameBoy* gb);
		~MemoryMap();

	private:
		void RenderContent();
		void OnEvent(Event event);
	
	private:
		Core::GameBoy* m_GameBoy;
	};
}
