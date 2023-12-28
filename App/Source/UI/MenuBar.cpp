#include "MenuBar.h"
#include "EventManager.h"

namespace App
{
	MenuBar::MenuBar() : ImguiWidgetBase("MenuBar")
	{
        disableTitleWrapper = true;
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
                    ExitPressed = true;
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
                if (ImGui::MenuItem("Debugger", "F5", &showDebugger))
                {
                    if (showDebugger)
                        EventManager::Instance().Emit(Event::DEBUGGER_ENABLE);
                    else
                        EventManager::Instance().Emit(Event::DEBUGGER_DISABLE);
                }

                if (ImGui::MenuItem("VRAM Viewer", "F6", &showVRAMViewer))
                {
                    if (showVRAMViewer)
                        EventManager::Instance().Emit(Event::VRAM_VIEWER_ENABLE);
                    else
                        EventManager::Instance().Emit(Event::VRAM_VIEWER_DISABLE);
                }

                if (ImGui::MenuItem("LCD", "F7", &showLCD))
                {
                    if (showLCD)
                        EventManager::Instance().Emit(Event::LCD_ENABLE);
                    else
                        EventManager::Instance().Emit(Event::LCD_DISABLE);
                }

                if (ImGui::MenuItem("Memory Map", "F8", &showMemoryMap))
                {
                    if (showMemoryMap)
                        EventManager::Instance().Emit(Event::MEMORY_MAP_ENABLE);
                    else
                        EventManager::Instance().Emit(Event::MEMORY_MAP_DISABLE);
                }

                if (ImGui::MenuItem("Audio Debugger", "F9", &showAudioDebugger))
                {
                    if (showAudioDebugger)
                        EventManager::Instance().Emit(Event::AUDIO_DEBUGGER_ENABLE);
                    else
                        EventManager::Instance().Emit(Event::AUDIO_DEBUGGER_DISABLE);
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