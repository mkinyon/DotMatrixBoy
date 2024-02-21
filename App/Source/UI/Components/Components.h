#pragma once

#include <imgui.h>

namespace UI
{
	class ToggleButton
	{
	public:
		ToggleButton()
		{
			m_Toggled = true;
		}

		ToggleButton(bool defaultToggleState)
		{
			m_Toggled = defaultToggleState;
		}

		bool Render(const char* label)
		{
			bool newState = m_Toggled;

			if (m_Toggled)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			}

			if (ImGui::Button(label))
			{
				newState = !m_Toggled;
			}

			if (m_Toggled)
			{
				ImGui::PopStyleColor(1);
			}

			return m_Toggled = newState;
		}

		bool m_Toggled = false;
	};
}