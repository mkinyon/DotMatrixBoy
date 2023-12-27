#pragma once

#include <imgui.h>
#include "SDL.h"

#include "Enums.h"

namespace App
{
	class ImguiWidgetBase
	{
	public:
		ImguiWidgetBase(const char* title) : title(title) {}

		// Function to render the window content
		virtual void RenderContent() = 0;

		// Function to render the entire window
		void Render()
		{
			if (!ShowWindow)
			{
				return;
			}

			if (!disableTitleWrapper) ImGui::Begin(title);

			RenderContent();

			if (!disableTitleWrapper) ImGui::End();
		}

		virtual void OnEvent(Event event) {};

	public:
		bool ShowWindow = true;
		bool disableTitleWrapper = false;

	private:
		const char* title;
	};
}


