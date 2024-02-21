#include "Console.h"
#include "EventManager.h"
#include "Core/Logger.h"
#include <deque>
#include <set>


namespace App
{
	Console::Console() : ImguiWidgetBase("Console"), m_VerboseButton(false), m_InfoButton(false)
	{
		EventManager::Instance().Subscribe(Event::CONSOLE_ENABLE, this);
		EventManager::Instance().Subscribe(Event::CONSOLE_DISABLE, this);
	}

	Console::~Console() {}

	void Console::RenderContent()
	{
		ImGui::BeginChild("LeftButtons", ImVec2(ImGui::GetContentRegionMax().x * 0.5f, 20), ImGuiChildFlags_None);
		m_VerboseButton.Render("Verbose"); ImGui::SameLine();
		m_InfoButton.Render("Info"); ImGui::SameLine();
		m_WarningButton.Render("Warning"); ImGui::SameLine();
		mErrorButton.Render("Error");
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("RightButtons", ImVec2(ImGui::GetContentRegionMax().x * 0.5f, 20), ImGuiChildFlags_None);
		m_AppButton.Render("APP"); ImGui::SameLine();
		m_APUButton.Render("APU"); ImGui::SameLine();
		m_CPUButton.Render("CPU"); ImGui::SameLine();
		m_MMUButton.Render("MMU"); ImGui::SameLine();
		m_PPUButton.Render("PPU");
		
		ImGui::EndChild();

		ImGui::Separator();

        if(ImGui::BeginChild("ScrollingRegion", ImVec2(0, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

			std::set<Core::Severity> filteredSeverities;
			if (m_VerboseButton.m_Toggled) filteredSeverities.insert(Core::Severity::Verbose);
			if (m_InfoButton.m_Toggled) filteredSeverities.insert(Core::Severity::Info);
			if (m_WarningButton.m_Toggled) filteredSeverities.insert(Core::Severity::Warning);
			if (mErrorButton.m_Toggled) filteredSeverities.insert(Core::Severity::Error);

			std::set<Core::Domain> filteredDomains;
			if (m_AppButton.m_Toggled) filteredDomains.insert(Core::Domain::APPLICATION);
			if (m_APUButton.m_Toggled) filteredDomains.insert(Core::Domain::APU);
			if (m_CPUButton.m_Toggled) filteredDomains.insert(Core::Domain::CPU);
			if (m_MMUButton.m_Toggled) filteredDomains.insert(Core::Domain::MMU);
			if (m_PPUButton.m_Toggled) filteredDomains.insert(Core::Domain::PPU);

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
			m_ShowWindow = true;
		}
		if (event == Event::CONSOLE_DISABLE)
		{
			m_ShowWindow = false;
		}
	}
}