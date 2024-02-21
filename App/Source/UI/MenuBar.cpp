#include "MenuBar.h"
#include "EventManager.h"

namespace App
{
	MenuBar::MenuBar() : ImguiWidgetBase("MenuBar")
	{
        m_DisableTitleWrapper = true;
	}

	MenuBar::~MenuBar() {}


	void MenuBar::RenderContent()
	{ 
        static bool isPaused = false;
        static bool enableBootRom = false;

        //ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 255, 255, 255));
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open Rom", "CTRL+O"))
                {
                    EventManager::Instance().Emit(Event::OPEN_FILE_DIALOG);
                }

                if (ImGui::MenuItem("Exit", "CTRL+Q"))
                {
                    m_ExitPressed = true;
                }

                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Emulator"))
            {
                if (ImGui::MenuItem("Pause", "CTRL+P", &isPaused)) {}
                if (ImGui::MenuItem("Reset", "CTRL+R")) {}
                if (ImGui::MenuItem("Step", "CTRL+S")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Enable Boot Rom", NULL, &enableBootRom)) {}
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools"))
            {
                if (ImGui::MenuItem("Debugger", "F5", &m_ShowDebugger))
                {
                    if (m_ShowDebugger)
                        EventManager::Instance().Emit(Event::DEBUGGER_ENABLE);
                    else
                        EventManager::Instance().Emit(Event::DEBUGGER_DISABLE);
                }

                if (ImGui::MenuItem("VRAM Viewer", "F6", &m_ShowVRAMViewer))
                {
                    if (m_ShowVRAMViewer)
                        EventManager::Instance().Emit(Event::VRAM_VIEWER_ENABLE);
                    else
                        EventManager::Instance().Emit(Event::VRAM_VIEWER_DISABLE);
                }

                if (ImGui::MenuItem("LCD", "F7", &m_ShowLCD))
                {
                    if (m_ShowLCD)
                        EventManager::Instance().Emit(Event::LCD_ENABLE);
                    else
                        EventManager::Instance().Emit(Event::LCD_DISABLE);
                }

                if (ImGui::MenuItem("Memory Map", "F8", &m_ShowMemoryMap))
                {
                    if (m_ShowMemoryMap)
                        EventManager::Instance().Emit(Event::MEMORY_MAP_ENABLE);
                    else
                        EventManager::Instance().Emit(Event::MEMORY_MAP_DISABLE);
                }

                if (ImGui::MenuItem("Audio Debugger", "F9", &m_ShowAudioDebugger))
                {
                    if (m_ShowAudioDebugger)
                        EventManager::Instance().Emit(Event::AUDIO_DEBUGGER_ENABLE);
                    else
                        EventManager::Instance().Emit(Event::AUDIO_DEBUGGER_DISABLE);
                }

                if (ImGui::MenuItem("Console", "F10", &m_ShowConsole))
                {
                    if (m_ShowConsole)
                        EventManager::Instance().Emit(Event::CONSOLE_ENABLE);
                    else
                        EventManager::Instance().Emit(Event::CONSOLE_DISABLE);
                }

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        //ImGui::PopStyleColor();
	}

    void MenuBar::OnEvent(Event event)
    {

    }
}