#pragma once

#include <imgui.h>
#include "SDL.h"

namespace App
{
	class ImguiWidgetBase
	{
	public:
		ImguiWidgetBase(const char* title) : title(title) {}

		// Function to render the window content
		virtual void RenderContent() = 0;

		// Function to render the entire window
		void Render() {
			ImGui::Begin(title);

			RenderContent();

			ImGui::End();
		}

	private:
		const char* title;
		
	};
}


