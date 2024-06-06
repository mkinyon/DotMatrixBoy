#pragma once

#include "ImguiWidgetBase.h"
#include "EventObserver.h"
#include "Core/GameBoy.h"
#include "Core/Defines.h"

namespace App
{
	class RomInfo : public ImguiWidgetBase, public EventObserver
	{
	public:
		RomInfo(Core::GameBoy* gb);
		~RomInfo();

	private:
		void RenderContent();
		void OnEvent(Event event);

	private:
		Core::GameBoy* m_GameBoy;
	};
}
