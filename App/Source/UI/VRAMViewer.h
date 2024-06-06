#pragma once

#include <stdint.h>

#include "SDL.h"
#include "ImguiWidgetBase.h"
#include "EventObserver.h"
#include "Core/GameBoy.h"

namespace App
{
	class VRAMViewer : public ImguiWidgetBase, public EventObserver
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
		Core::GameBoy* m_GameBoy;
		SDL_Texture* m_TilesTexture;
		uint8_t m_TilesTexturePixels[128 * 192 * 4] {};

		enum class WindowState
		{
			Tiles,
			BGMaps,
			OAM
		};
	};
}

