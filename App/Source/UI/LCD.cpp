#include "LCD.h"
#include "EventManager.h"


namespace App
{
	LCD::LCD(uint8_t* lcdPixels, SDL_Renderer* renderer) : ImguiWidgetBase("LCD")
	{
		EventManager::Instance().Subscribe(Event::LCD_ENABLE, this);
		EventManager::Instance().Subscribe(Event::LCD_DISABLE, this);

		m_Texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
		m_Pixels = lcdPixels;
	}

	LCD::~LCD()
	{
		SDL_DestroyTexture(m_Texture);
	}

	void LCD::RenderContent()
	{
		ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		// Map lcd_pixels to pixels
		for (int y = 0; y < 144; ++y) {
			for (int x = 0; x < 160; ++x) {
				// Get the color value from lcd_pixels
				uint8_t colorId = m_Pixels[y * 160 + x];

				// Map the color value to the corresponding block in pixels
				int blockIndex = (y * 160 + x) * 4;
				if (colorId == 0)
				{
					m_TexturePixels[blockIndex] = 255;     // a
					m_TexturePixels[blockIndex + 1] = 15;  // b
					m_TexturePixels[blockIndex + 2] = 188; // g
					m_TexturePixels[blockIndex + 3] = 155; // r
				}
				if (colorId == 1)
				{
					m_TexturePixels[blockIndex] = 255;
					m_TexturePixels[blockIndex + 1] = 15;
					m_TexturePixels[blockIndex + 2] = 172;
					m_TexturePixels[blockIndex + 3] = 139;
				}
				if (colorId == 2)
				{
					m_TexturePixels[blockIndex] = 255;
					m_TexturePixels[blockIndex + 1] = 48;
					m_TexturePixels[blockIndex + 2] = 98;
					m_TexturePixels[blockIndex + 3] = 48;
				}
				if (colorId == 3)
				{
					m_TexturePixels[blockIndex] = 255;
					m_TexturePixels[blockIndex + 1] = 15;
					m_TexturePixels[blockIndex + 2] = 56;
					m_TexturePixels[blockIndex + 3] = 15;
				}
			}
		}

		SDL_UpdateTexture(m_Texture, NULL, m_TexturePixels, 160 * 4);

		ImVec2 contentSize = ImGui::GetContentRegionAvail();
		float originalWidth = 160.0f; 
		float originalHeight = 144.0f;

		float scaleX = contentSize.x / originalWidth;
		float scaleY = contentSize.y / originalHeight;

		float minScale = std::min(scaleX, scaleY);

		ImVec2 imageSize(originalWidth * minScale, originalHeight * minScale);

		ImGui::Image(reinterpret_cast<ImTextureID>(m_Texture), imageSize);
	}

	void LCD::OnEvent(Event event)
	{
		if (event == Event::LCD_ENABLE)
		{
			m_ShowWindow = true;
		}
		if (event == Event::LCD_DISABLE)
		{
			m_ShowWindow = false;
		}
	}
}