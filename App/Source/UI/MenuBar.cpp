#include "MenuBar.h"

namespace App
{
	MenuBar::MenuBar() : ImguiWidgetBase("MemoryMap")
	{
	}

	MenuBar::~MenuBar() {}


	void MenuBar::RenderContent()
	{
        static bool isPaused = false;
        static bool enableBootRom = false;
        static bool showDebugger = true;
        static bool showVRAMViewer = true;
        static bool showLCD = true;
        static bool showMemoryMap = true;

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open Rom", "CTRL+O")) {}
                if (ImGui::MenuItem("Exit", "CTRL+Q")) {}
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
                if (ImGui::MenuItem("Debugger", "CTRL+F5", &showDebugger)) {}
                if (ImGui::MenuItem("VRAM Viewer", "CTRL+F6", &showVRAMViewer)) {}
                if (ImGui::MenuItem("LCD", "CTRL+F7", &showLCD)) {}
                if (ImGui::MenuItem("Memory Map", "CTRL+F8", &showMemoryMap)) {}
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
	}
}