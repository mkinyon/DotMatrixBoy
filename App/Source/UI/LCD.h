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
		SDL_Texture* m_Texture;
		uint8_t* m_Pixels {};
		uint8_t m_TexturePixels[160 * 144 * 4] {};
	};
}

