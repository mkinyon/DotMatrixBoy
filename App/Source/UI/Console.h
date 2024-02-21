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

		UI::ToggleButton m_VerboseButton;
		UI::ToggleButton m_InfoButton;
		UI::ToggleButton m_WarningButton;
		UI::ToggleButton mErrorButton;

		UI::ToggleButton m_AppButton;
		UI::ToggleButton m_APUButton;
		UI::ToggleButton m_CPUButton;
		UI::ToggleButton m_MMUButton;
		UI::ToggleButton m_PPUButton;
	};
}

