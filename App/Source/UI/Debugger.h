#pragma once

#include "ImguiWindowBase.h"
#include "Core/GameBoy.h"

namespace App
{
	class Debugger : public ImguiWindowBase
	{
	public:
		Debugger(Core::GameBoy& gb);
		~Debugger();

	private:
		void RenderContent();
	
	private:
		Core::GameBoy gameboy;
	
	};
}
