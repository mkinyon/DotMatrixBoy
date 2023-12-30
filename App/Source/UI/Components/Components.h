#pragma once

#include <imgui.h>

namespace UI
{
	class ToggleButton
	{
	public:
		ToggleButton()
		{
			Toggled = true;
		}

		ToggleButton(bool defaultToggleState)
		{
			Toggled = defaultToggleState;
		}

		bool Render(const char* label)
		{
			bool newState = Toggled;

			if (Toggled)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
			}

			if (ImGui::Button(label))
			{
				newState = !Toggled;
			}

			if (Toggled)
			{
				ImGui::PopStyleColor(1);
			}

			return Toggled = newState;
		}

		bool Toggled = false;
	};
}