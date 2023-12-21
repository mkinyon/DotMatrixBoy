#pragma once

#include "SDL.h"

namespace App
{
	class Window
	{
	public:
		Window(int screenWidth, int screenHeight, const char* windowTitle);
		~Window();

	public:
		bool Initialize();
		void Update(bool& done);
		void BeginRender();
		void EndRender();

		SDL_Renderer* GetRenderer();
		Uint32 GetElapsedTime();

	private:
		SDL_Window* window;
		SDL_Renderer* renderer;

		Uint32 startTime;
		Uint32 elapsedTime;
	};
}

