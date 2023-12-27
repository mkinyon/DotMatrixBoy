#pragma once

#include <stdint.h>

#include "SDL.h"
#include "imgui.h"
#include "ImguiWidgetBase.h"

namespace App
{
	class LCD : public ImguiWidgetBase
	{
	public:
		LCD(uint8_t* lcdPixels, SDL_Renderer* renderer);
		~LCD();

	private:
		void RenderContent();
		void OnEvent(Event event);

	private:
		SDL_Texture* texture;
		uint8_t* pixels {};
		uint8_t texturePixels[160 * 144 * 4] {};
	};
}

