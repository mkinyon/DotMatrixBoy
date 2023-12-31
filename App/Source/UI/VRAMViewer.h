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

		void RenderTilesView();
		void RenderBGMapsView();
		void RenderOAMView();
		void updateTexture();

	private:
		Core::GameBoy* gameboy;
		SDL_Texture* tilesTexture;
		uint8_t tilesTexturePixels[128 * 192 * 4] {};

		enum class WindowState
		{
			Tiles,
			BGMaps,
			OAM
		};
	};
}

