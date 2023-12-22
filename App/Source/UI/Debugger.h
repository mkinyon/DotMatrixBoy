#pragma once

#include "ImguiWidgetBase.h"
#include "Core/GameBoy.h"
#include "Core/Defines.h"

namespace App
{
	class Debugger : public ImguiWidgetBase
	{
	public:
		Debugger(Core::GameBoy& gb);
		~Debugger();

	private:
		void RenderContent();
	
	private:
		Core::GameBoy& gameboy;
		std::map<uint16_t, std::string> instructions;
	};
}
