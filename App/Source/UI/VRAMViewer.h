#pragma once

#include <stdint.h>

#include "SDL.h"
#include "imgui.h"
#include "ImguiWidgetBase.h"
#include "Core/GameBoy.h"

namespace App
{
	class VRAMViewer : public ImguiWidgetBase
	{
	public:
		VRAMViewer(Core::GameBoy& gb, SDL_Renderer* renderer);
		~VRAMViewer();

	private:
		void RenderContent();

	private:
		Core::GameBoy& gameboy;
		SDL_Texture* texture;
		uint8_t texturePixels[128 * 192 * 4] {};
	};
}

