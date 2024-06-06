#pragma once

#include "SDL.h"
#include "Core/GameBoy.h"
#include "AppState.h"
#include "UI/ImguiWidgetBase.h"
#include "EventObserver.h"
#include <mutex>

namespace App
{
	class Window : public EventObserver
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

		void AttachWidget(std::unique_ptr<ImguiWidgetBase> widget);
		const std::vector<std::unique_ptr<ImguiWidgetBase>>& GetWidgets() const;

		void OnEvent(Event event);
		bool ShouldExit();

	private:
		sAppState& m_AppState;
		std::vector<std::unique_ptr<ImguiWidgetBase>> m_Widgets;
		bool m_ShouldExit = false;

		SDL_Window* m_SDLWindow;
		SDL_Renderer* m_SDLRenderer;

		Core::GameBoy* m_GameBoy;

		Uint64 m_StartTime;
		Uint64 m_ElapsedTime;
	};
}

