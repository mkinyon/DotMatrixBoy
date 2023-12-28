#pragma once

#include "ImguiWidgetBase.h"
#include "Core/GameBoy.h"
#include "Core/Defines.h"

namespace App
{
	class AudioDebugger : public ImguiWidgetBase
	{
	public:
		AudioDebugger(Core::GameBoy* gb);
		~AudioDebugger();

	private:
		void RenderContent();
		void OnEvent(Event event);

	private:
		Core::GameBoy* gameboy;
	};
}
