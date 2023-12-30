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
		infoButton.Render("Info"); ImGui::SameLine();
		errorButton.Render("Error"); ImGui::SameLine();
		cpuButton.Render("CPU"); ImGui::SameLine();
		ppuButton.Render("PPU"); ImGui::SameLine();
		mmuButton.Render("MMU");

		ImGui::Separator();

        if(ImGui::BeginChild("ScrollingRegion", ImVec2(0, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

            std::vector<Core::LogMessage> items = Core::Logger::Instance().GetMessages();
			reverse(items.begin(), items.end()); // TODO: need to make this faster
            for (Core::LogMessage item : items)
            {
				switch (item.type)
				{
				case Core::LogMessageType::Info:
					if (infoButton.Toggled) {
						ImGui::TextUnformatted("[INFO] "); ImGui::SameLine(); ImGui::TextUnformatted(item.message.c_str());
					}
					break;
				case Core::LogMessageType::Error:
					if (errorButton.Toggled) {
						ImGui::TextUnformatted("[ERROR] "); ImGui::SameLine(); ImGui::TextUnformatted(item.message.c_str());
					}
					break;
				case Core::LogMessageType::CPU:
					if (cpuButton.Toggled) {
						ImGui::TextUnformatted("[CPU] "); ImGui::SameLine(); ImGui::TextUnformatted(item.message.c_str());
					}
					break;
				case Core::LogMessageType::PPU:
					if (ppuButton.Toggled) {
						ImGui::TextUnformatted("[PPU] "); ImGui::SameLine(); ImGui::TextUnformatted(item.message.c_str());
					}
					break;
				case Core::LogMessageType::MMU:
					if (mmuButton.Toggled) {
						ImGui::TextUnformatted("[MMU] "); ImGui::SameLine(); ImGui::TextUnformatted(item.message.c_str());
					}
					break;
				}
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