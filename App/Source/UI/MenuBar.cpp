#include "MenuBar.h"
#include "EventManager.h"

namespace App
{
	MenuBar::MenuBar(Core::GameBoy* gb, sAppState& appState) : ImguiWidgetBase("MenuBar"), m_GameBoy(gb), m_AppState(appState)
	{
        m_DisableTitleWrapper = true;
	}

	MenuBar::~MenuBar() {}

	void MenuBar::RenderContent()
	{ 
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open Rom", "CTRL+O"))
                {
                    EventManager::Instance().Emit(Event::OPEN_FILE_DIALOG);
                }

                ImGui::Separator();

				for (auto& rom : m_AppState.recentRoms)
				{
					if (ImGui::MenuItem(rom.c_str(), ""))
					{
						m_GameBoy->LoadRom(rom.c_str());
						m_AppState.AddRecentRomEntry(rom);
						m_AppState.SaveStateToFile();
					}
				}

                ImGui::Separator();

                if (ImGui::MenuItem("Exit", "CTRL+Q"))
                {
                    m_ExitPressed = true;
                    EventManager::Instance().Emit(Event::MENU_BAR_EXIT);
                }

                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Emulator"))
            {
                if (ImGui::MenuItem("Pause", "CTRL+P", &m_AppState.IsPaused)) {}
                if (ImGui::MenuItem("Step", "CTRL+S")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Enable Boot Rom", NULL, &m_AppState.IsBootRomEnabled)) {}
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools"))
            {
                if (ImGui::MenuItem("Debugger", "F5", &m_AppState.ShowDebugger))
                {
                    if (m_AppState.ShowDebugger)
                        EventManager::Instance().Emit(Event::DEBUGGER_ENABLE);
                    else
                        EventManager::Instance().Emit(Event::DEBUGGER_DISABLE);
                }

                if (ImGui::MenuItem("VRAM Viewer", "F6", &m_AppState.ShowVRAMViewer))
                {
                    if (m_AppState.ShowVRAMViewer)
                        EventManager::Instance().Emit(Event::VRAM_VIEWER_ENABLE);
                    else
                        EventManager::Instance().Emit(Event::VRAM_VIEWER_DISABLE);
                }

                if (ImGui::MenuItem("LCD", "F7", &m_AppState.ShowLCD))
                {
                    if (m_AppState.ShowLCD)
                        EventManager::Instance().Emit(Event::LCD_ENABLE);
                    else
                        EventManager::Instance().Emit(Event::LCD_DISABLE);
                }

                if (ImGui::MenuItem("Memory Map", "F8", &m_AppState.ShowMemoryMap))
                {
                    if (m_AppState.ShowMemoryMap)
                        EventManager::Instance().Emit(Event::MEMORY_MAP_ENABLE);
                    else
                        EventManager::Instance().Emit(Event::MEMORY_MAP_DISABLE);
                }

                if (ImGui::MenuItem("Audio Debugger", "F9", &m_AppState.ShowAudioDebugger))
                {
                    if (m_AppState.ShowAudioDebugger)
                        EventManager::Instance().Emit(Event::AUDIO_DEBUGGER_ENABLE);
                    else
                        EventManager::Instance().Emit(Event::AUDIO_DEBUGGER_DISABLE);
                }

                if (ImGui::MenuItem("Console", "F10", &m_AppState.ShowConsole))
                {
                    if (m_AppState.ShowConsole)
                        EventManager::Instance().Emit(Event::CONSOLE_ENABLE);
                    else
                        EventManager::Instance().Emit(Event::CONSOLE_DISABLE);
                }

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
	}

    void MenuBar::OnEvent(Event event)
    {

    }
}