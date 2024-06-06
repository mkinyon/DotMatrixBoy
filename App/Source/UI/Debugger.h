#pragma once

#include "ImguiWidgetBase.h"
#include "EventObserver.h"
#include "Core/GameBoy.h"
#include "Core/Defines.h"

namespace App
{
	class Debugger : public ImguiWidgetBase, public EventObserver
	{
	public:
		Debugger(Core::GameBoy* gb);
		~Debugger();

	private:
		void RenderContent();
		void OnEvent(Event event);
	
	private:
		Core::GameBoy* m_GameBoy;
		std::map<uint16_t, std::string> m_Instructions;
	};
}
