#pragma once

#include <stdint.h>

#include "SDL.h"
#include "ImguiWidgetBase.h"
#include "Core/GameBoy.h"

namespace App
{
	class VRAMViewer : public ImguiWidgetBase
	{
	public:
		VRAMViewer(Core::GameBoy* gb, SDL_Renderer* renderer);
		~VRAMViewer();

	private:
		void RenderContent();
		void OnEvent(Event event);

	private:
		Core::GameBoy* gameboy;
		SDL_Texture* texture;
		uint8_t texturePixels[128 * 192 * 4] {};
	};
}

