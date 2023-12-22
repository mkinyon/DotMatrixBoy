#include "VRAMViewer.h"
#include <algorithm>


namespace App
{
	VRAMViewer::VRAMViewer(Core::GameBoy& gb, SDL_Renderer* renderer) : ImguiWidgetBase("VRAM Viewer"), gameboy(gb)
	{
		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 192);
	}

	VRAMViewer::~VRAMViewer()
	{
		SDL_DestroyTexture(texture);
	}

	void VRAMViewer::RenderContent()
	{
		int x = 8;
		int y = 0;

		int count = 0;
		for (uint16_t byte = 0x8000; byte <= 0x97FF; byte += 2)
		{
			uint8_t firstByte = gameboy.ReadFromMemoryMap(byte);
			uint8_t secondByte = gameboy.ReadFromMemoryMap(byte + 1);
			for (int iBit = 0; iBit < 8; iBit++)
			{
				uint8_t firstBit = (firstByte >> iBit) & 0x01;
				uint8_t secondBit = (secondByte >> iBit) & 0x01;
				int colorIndex = (secondBit << 1) | firstBit;
		
				int draw_x = x - iBit; // Adjusted x coordinate for drawing
				int draw_y = y + count; // Adjusted y coordinate for drawing
		
				// get the background palette
				uint8_t bgPalette = gameboy.ReadFromMemoryMap(Core::HW_BGP_BG_PALETTE_DATA);
				uint8_t colorId = bgPalette >> (colorIndex * 2) & 0x03;
		
				int blockIndex = (draw_y * 128 + draw_x) * 4;
				
				//_ASSERT(blockIndex < 0);

				if (colorId == 0)
				{
					texturePixels[blockIndex] = 255;     // a
					texturePixels[blockIndex + 1] = 15;  // b
					texturePixels[blockIndex + 2] = 188; // g
					texturePixels[blockIndex + 3] = 155; // r
				}
				if (colorId == 1)
				{
					texturePixels[blockIndex] = 255;
					texturePixels[blockIndex + 1] = 15;
					texturePixels[blockIndex + 2] = 172;
					texturePixels[blockIndex + 3] = 139;
				}
				if (colorId == 2)
				{
					texturePixels[blockIndex] = 255;
					texturePixels[blockIndex + 1] = 48;
					texturePixels[blockIndex + 2] = 98;
					texturePixels[blockIndex + 3] = 48;
				}
				if (colorId == 3)
				{
					texturePixels[blockIndex] = 255;
					texturePixels[blockIndex + 1] = 15;
					texturePixels[blockIndex + 2] = 56;
					texturePixels[blockIndex + 3] = 15;
				}
			}
			count++;
		
			if ((count % 128) == 0)
			{
				x -= 120;
			}
			else if ((count % 8) == 0)
			{
				x += 8; // Move x coordinate right by 8
				y -= 8;
			}
		}

		SDL_UpdateTexture(texture, NULL, texturePixels, 128 * 4);

		ImVec2 contentSize = ImGui::GetContentRegionAvail();
		float originalWidth = 128.0f; 
		float originalHeight = 192.0f;

		float scaleX = contentSize.x / originalWidth;
		float scaleY = contentSize.y / originalHeight;

		float minScale = std::min(scaleX, scaleY);

		ImVec2 imageSize(originalWidth * minScale, originalHeight * minScale);

		ImGui::Image(reinterpret_cast<ImTextureID>(texture), imageSize);
	}
}