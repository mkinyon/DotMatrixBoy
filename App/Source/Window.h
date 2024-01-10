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
		SDL_Window* sdlWindow;
		SDL_Renderer* sdlRenderer;
		SDL_AudioDeviceID sdlAudioDevice;

		Core::GameBoy* gameboy;

		Uint32 startTime;
		Uint32 elapsedTime;
	};
}

