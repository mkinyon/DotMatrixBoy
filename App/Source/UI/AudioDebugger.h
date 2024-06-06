#pragma once

#include "ImguiWidgetBase.h"
#include "EventObserver.h"
#include "Core/GameBoy.h"
#include "Core/Defines.h"

namespace App
{
	class AudioDebugger : public ImguiWidgetBase, public EventObserver
	{
	public:
		AudioDebugger(Core::GameBoy* gb);
		~AudioDebugger();

	private:
		void RenderContent();
		void OnEvent(Event event);

	private:
		Core::GameBoy* m_GameBoy;
	};
}
