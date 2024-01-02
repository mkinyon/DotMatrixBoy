#include "Console.h"
#include "EventManager.h"
#include "Core/Logger.h"
#include <deque>
#include <set>


namespace App
{
	Console::Console() : ImguiWidgetBase("Console"), verboseButton(false), infoButton(false)
	{
		EventManager::Instance().Subscribe(Event::CONSOLE_ENABLE, this);
		EventManager::Instance().Subscribe(Event::CONSOLE_DISABLE, this);
	}

	Console::~Console() {}

	void Console::RenderContent()
	{
		ImGui::BeginChild("LeftButtons", ImVec2(ImGui::GetContentRegionMax().x * 0.5f, 20), ImGuiChildFlags_None);
		verboseButton.Render("Verbose"); ImGui::SameLine();
		infoButton.Render("Info"); ImGui::SameLine();
		warningButton.Render("Warning"); ImGui::SameLine();
		errorButton.Render("Error");
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("RightButtons", ImVec2(ImGui::GetContentRegionMax().x * 0.5f, 20), ImGuiChildFlags_None);
		appButton.Render("APP"); ImGui::SameLine();
		apuButton.Render("APU"); ImGui::SameLine();
		cpuButton.Render("CPU"); ImGui::SameLine();
		mmuButton.Render("MMU"); ImGui::SameLine();
		ppuButton.Render("PPU");
		
		ImGui::EndChild();

		ImGui::Separator();

        if(ImGui::BeginChild("ScrollingRegion", ImVec2(0, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

			std::set<Core::Severity> filteredSeverities;
			if (verboseButton.Toggled) filteredSeverities.insert(Core::Severity::Verbose);
			if (infoButton.Toggled) filteredSeverities.insert(Core::Severity::Info);
			if (warningButton.Toggled) filteredSeverities.insert(Core::Severity::Warning);
			if (errorButton.Toggled) filteredSeverities.insert(Core::Severity::Error);

			std::set<Core::Domain> filteredDomains;
			if (appButton.Toggled) filteredDomains.insert(Core::Domain::APPLICATION);
			if (apuButton.Toggled) filteredDomains.insert(Core::Domain::APU);
			if (cpuButton.Toggled) filteredDomains.insert(Core::Domain::CPU);
			if (mmuButton.Toggled) filteredDomains.insert(Core::Domain::MMU);
			if (ppuButton.Toggled) filteredDomains.insert(Core::Domain::PPU);

            std::deque<Core::Message> messages = Core::Logger::Instance().GetMessages();
            for (Core::Message msg : messages)
            {
				if (filteredSeverities.count(msg.severity) > 0 && filteredDomains.count(msg.domain) > 0)
				{
					ImGui::TextUnformatted(msg.message.c_str());
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