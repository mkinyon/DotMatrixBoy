#pragma once

#include <imgui.h>
#include "SDL.h"

#include "Enums.h"

namespace App
{
	class ImguiWidgetBase
	{
	public:
		ImguiWidgetBase(const char* title) : m_Title(title) {}

		// Function to render the window content
		virtual void RenderContent() = 0;

		// Function to render the entire window
		void Render()
		{
			if (!m_ShowWindow)
			{
				return;
			}

			if (!m_DisableTitleWrapper) ImGui::Begin(m_Title);

			RenderContent();

			if (!m_DisableTitleWrapper) ImGui::End();
		}

		virtual void OnEvent(Event event) {};

	public:
		bool m_ShowWindow = true;
		bool m_DisableTitleWrapper = false;

	private:
		const char* m_Title;
	};
}


