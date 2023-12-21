#pragma once

#include <stdint.h>

#include "SDL.h"
#include "imgui.h"

#include "ImguiWindowBase.h"

namespace App
{
	class LCD : public ImguiWindowBase
	{
	public:
		LCD(uint8_t* lcdPixels, SDL_Renderer* renderer);
		~LCD();

	public:
		void RenderContent();

	private:
		uint8_t* pixels {};
		uint8_t texturePixels[160 * 144 * 4] {};
	};
}

