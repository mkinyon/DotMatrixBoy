#include "LCDWindow.h"



namespace App
{
	SDL_Texture* texture;

	LCDWindow::LCDWindow(SDL_Renderer* renderer)
	{
		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
	}

	LCDWindow::~LCDWindow()
	{
		SDL_DestroyTexture(texture);
	}

	void LCDWindow::Render(uint8_t* pixels)
	{
		SDL_UpdateTexture(texture, NULL, pixels, 160);

		ImGui::Begin("Your Window Title");

		// Calculate the size of the image within the ImGui window
		ImVec2 imageSize(static_cast<float>(160), static_cast<float>(144));

		// Render the texture inside the ImGui window
		ImGui::Image(reinterpret_cast<ImTextureID>(texture), imageSize);

		ImGui::End();
	}
}