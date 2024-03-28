#pragma once

#include "SDL.h"
#include "Core/GameBoy.h"
#include "AppState.h"
#include <mutex>

namespace App
{
	class Window
	{
	public:
		Window(int screenWidth, int screenHeight, const char* windowTitle, Core::GameBoy* gb, sAppState& appState);
		~Window();

	public:
		bool Initialize();
		void Update(bool& isRunning);
		void BeginRender();
		void EndRender();

		SDL_Renderer* GetRenderer();
		Uint64 GetElapsedTime();

	private:
		sAppState& m_AppState;

		SDL_Window* m_SDLWindow;
		SDL_Renderer* m_SDLRenderer;

		Core::GameBoy* m_GameBoy;

		Uint64 m_StartTime;
		Uint64 m_ElapsedTime;
	};
}

