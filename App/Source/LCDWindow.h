#pragma once

#include <stdint.h>

#include "SDL.h"
#include "imgui.h"

namespace App
{
	class LCDWindow
	{
	public:
		LCDWindow(SDL_Renderer* renderer);
		~LCDWindow();

	public:
		void Render(uint8_t* pixels);
	};
}

