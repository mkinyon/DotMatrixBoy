#pragma once

#include "ImguiWidgetBase.h"
#include "Enums.h"

namespace App
{
	class MenuBar : public ImguiWidgetBase
	{
	public:
		MenuBar();
		~MenuBar();

	public:
		bool m_ExitPressed = false;

	private:
		void RenderContent();
		void OnEvent(Event event);

	private:
		bool m_ShowMemoryMap;
		bool m_ShowDebugger;
		bool m_ShowVRAMViewer;
		bool m_ShowLCD;
		bool m_ShowAudioDebugger;
		bool m_ShowConsole;
	};
}
