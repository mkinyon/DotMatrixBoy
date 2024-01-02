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

		UI::ToggleButton verboseButton;
		UI::ToggleButton infoButton;
		UI::ToggleButton warningButton;
		UI::ToggleButton errorButton;

		UI::ToggleButton appButton;
		UI::ToggleButton apuButton;
		UI::ToggleButton cpuButton;
		UI::ToggleButton mmuButton;
		UI::ToggleButton ppuButton;
	};
}

