#pragma once

#include "SDL.h"
#include "Core/GameBoy.h"
#include <mutex>

namespace App
{
	class Window
	{
	public:
		Window(int screenWidth, int screenHeight, const char* windowTitle, Core::GameBoy* gb);
		~Window();

		//std::mutex mutex;

	public:
		bool Initialize();
		void Update(bool& isRunning);
		void BeginRender();
		void EndRender();

		SDL_Renderer* GetRenderer();
		Uint32 GetElapsedTime();

		static void StaticAudioCallback(void* userdata, Uint8* stream, int len);

	private:
		SDL_Window* m_SDLWindow;
		SDL_Renderer* m_SDLRenderer;
		SDL_AudioDeviceID m_SDLAudioDevice;

		Core::GameBoy* m_GameBoy;

		Uint32 m_StartTime;
		Uint32 m_ElapsedTime;
	};
}

