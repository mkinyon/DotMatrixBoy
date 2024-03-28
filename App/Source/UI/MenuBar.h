#pragma once

#include "ImguiWidgetBase.h"
#include "Enums.h"
#include "AppState.h"

namespace App
{
	class MenuBar : public ImguiWidgetBase
	{
	public:
		MenuBar(sAppState& appState);
		~MenuBar();

	public:
		bool m_ExitPressed = false;

	private:
		void RenderContent();
		void OnEvent(Event event);

	private:
		sAppState& m_AppState;
	};
}
