#include "LCD.h"
#include <algorithm>


namespace App
{
	SDL_Texture* texture;

	LCD::LCD(uint8_t* lcdPixels, SDL_Renderer* renderer) : ImguiWindowBase("LCD")
	{
		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
		pixels = lcdPixels;
	}

	LCD::~LCD()
	{
		SDL_DestroyTexture(texture);
	}

	void LCD::RenderContent()
	{
		// Map lcd_pixels to pixels
		for (int y = 0; y < 144; ++y) {
			for (int x = 0; x < 160; ++x) {
				// Get the color value from lcd_pixels
				uint8_t colorId = pixels[y * 160 + x];

				// Map the color value to the corresponding block in pixels
				int blockIndex = (y * 160 + x) * 4;
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
		}

		SDL_UpdateTexture(texture, NULL, texturePixels, 160 * 4);

		ImVec2 contentSize = ImGui::GetContentRegionAvail();
		float originalWidth = 160.0f; 
		float originalHeight = 144.0f;

		float scaleX = contentSize.x / originalWidth;
		float scaleY = contentSize.y / originalHeight;

		float minScale = std::min(scaleX, scaleY);

		ImVec2 imageSize(originalWidth * minScale, originalHeight * minScale);

		ImGui::Image(reinterpret_cast<ImTextureID>(texture), imageSize);
	}
}