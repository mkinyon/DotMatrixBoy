#include "Console.h"
#include "EventManager.h"
#include "Core/Logger.h"


namespace App
{
	Console::Console() : ImguiWidgetBase("Console")
	{
		EventManager::Instance().Subscribe(Event::CONSOLE_ENABLE, this);
		EventManager::Instance().Subscribe(Event::CONSOLE_DISABLE, this);
	}

	Console::~Console() {}

	void Console::RenderContent()
	{
		ImGui::Button("Info"); ImGui::SameLine();
		ImGui::Button("Error"); ImGui::SameLine();
		ImGui::Button("CPU"); ImGui::SameLine();
		ImGui::Button("PPU"); ImGui::SameLine();
		ImGui::Button("MMU");

		ImGui::Separator();

        if(ImGui::BeginChild("ScrollingRegion", ImVec2(0, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

            std::vector<Core::LogMessage> items = Core::Logger::Instance().GetMessages();
			reverse(items.begin(), items.end()); // TODO: need to make this faster
            for (Core::LogMessage item : items)
            {
				// TODO: wire up message type filtering

				switch (item.type)
				{
				case Core::LogMessageType::Info:
					ImGui::TextUnformatted("[INFO] "); ImGui::SameLine();
					break;
				case Core::LogMessageType::Error:
					ImGui::TextUnformatted("[ERROR] "); ImGui::SameLine();
					break;
				case Core::LogMessageType::CPU:
					ImGui::TextUnformatted("[CPU] "); ImGui::SameLine();
					break;
				case Core::LogMessageType::PPU:
					ImGui::TextUnformatted("[PPU] "); ImGui::SameLine();
					break;
				case Core::LogMessageType::MMU:
					ImGui::TextUnformatted("[MMU] "); ImGui::SameLine();
					break;
				}

                ImGui::TextUnformatted(item.message.c_str());
            }

			ImGui::SetScrollHereY(1.0f);
            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
	}

	void Console::OnEvent(Event event)
	{
		if (event == Event::CONSOLE_ENABLE)
		{
			ShowWindow = true;
		}
		if (event == Event::CONSOLE_DISABLE)
		{
			ShowWindow = false;
		}
	}
}