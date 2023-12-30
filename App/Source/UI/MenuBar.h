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
		bool ExitPressed = false;

	private:
		void RenderContent();
		void OnEvent(Event event);

	private:
		bool showMemoryMap;
		bool showDebugger;
		bool showVRAMViewer;
		bool showLCD;
		bool showAudioDebugger;
		bool showConsole;
	};
}
