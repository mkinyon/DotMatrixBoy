#pragma once
#include "Core/GameBoy.h"
#include "ImguiWidgetBase.h"
#include "Enums.h"
#include "AppState.h"

namespace App
{
	class MenuBar : public ImguiWidgetBase
	{
	public:
		MenuBar(Core::GameBoy* gb, sAppState& appState);
		~MenuBar();

	public:
		bool m_ExitPressed = false;

	private:
		void RenderContent();
		void OnEvent(Event event);

	private:
		Core::GameBoy* m_GameBoy;
		sAppState& m_AppState;
	};
}
