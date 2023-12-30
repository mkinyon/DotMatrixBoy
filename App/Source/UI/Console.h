#pragma once

#include "imgui.h"
#include "ImguiWidgetBase.h"
#include "UI/Components/Components.h"

namespace App
{
	class Console : public ImguiWidgetBase
	{
	public:
		Console();
		~Console();

	private:
		void RenderContent();
		void OnEvent(Event event);

		UI::ToggleButton infoButton;
		UI::ToggleButton errorButton;
		UI::ToggleButton cpuButton;
		UI::ToggleButton ppuButton;
		UI::ToggleButton mmuButton;
	};
}

