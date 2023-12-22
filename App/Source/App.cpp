
#include "Core\GameBoy.h"
#include "Core\Cpu.h"
#include "Core\Cartridge.h"
#include "Core\Defines.h"
#include "Core\Utils.h"

#include <vector>
#include <algorithm>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <stdio.h>
#include <SDL.h>

#include "Window.h"
#include "UI/MenuBar.h"
#include "UI/LCD.h"
#include "UI/Debugger.h"
#include "UI/MemoryMap.h"
#include "UI/VRAMViewer.h"

App::Window window(1280, 720, "DotMatrixBoy");

Core::GameBoy gb;
std::shared_ptr<Core::Cartridge> cart;
bool isPaused = true;
bool enableBootRom = false;
//const char* romName = "../Roms/hello-world.gb";
//const char* romName = "../Roms/02-interrupts.gb";
const char* romName = "../Roms/tetris.gb";

// Main code
int main(int, char**)
{
    window.Initialize();

    cart = std::make_shared<Core::Cartridge>(romName, enableBootRom);
    gb.InsertCartridge(*cart);
    gb.Run(enableBootRom);
    
    // Widgets
    App::MenuBar* menuBar = new App::MenuBar();
    App::LCD* lcdWindow = new App::LCD(gb.ppu.m_lcdPixels, window.GetRenderer());
    App::Debugger* debugger = new App::Debugger(gb);
    App::MemoryMap* memoryMap = new App::MemoryMap(gb);
    App::VRAMViewer* vramViewer = new App::VRAMViewer(gb, window.GetRenderer());

    bool show_demo_window = true;
    bool show_another_window = false;

    // Main loop
    bool isRunning = true;
    while (isRunning)
    {
        window.Update(isRunning, gb);

        gb.Clock((float)std::min((int)window.GetElapsedTime(), 16 ));
        //gb.Clock(16);

        window.BeginRender();

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // render widgets
        menuBar->Render();
        lcdWindow->Render();
        debugger->Render();
        memoryMap->Render();
        vramViewer->Render();

        window.EndRender();
    }

    return 0;
}
