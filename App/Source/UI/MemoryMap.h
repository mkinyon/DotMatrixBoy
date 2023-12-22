#pragma once

#include "ImguiWidgetBase.h"
#include "Core/GameBoy.h"
#include "Core/Defines.h"

namespace App
{
	class MemoryMap : public ImguiWidgetBase
	{
	public:
		MemoryMap(Core::GameBoy& gb);
		~MemoryMap();

	private:
		void RenderContent();
	
	private:
		Core::GameBoy& gameboy;
	};
}
