
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "Window.h"
#include "AppState.h"

#include "Core\Cartridge.h"
#include "Core\GameBoy.h"

#include "UI/AudioDebugger.h"
#include "UI/Console.h"
#include "UI/Debugger.h"
#include "UI/FileDialog.h"
#include "UI/LCD.h"
#include "UI/MemoryMap.h"
#include "UI/MenuBar.h"
#include "UI/RomInfo.h"
#include "UI/VRAMViewer.h"

using namespace App;

int main(int argc, char* argv[])
{
    bool isRunning = true;

    sAppState appState;
    appState.IsPaused = true; // need to always set to true since we don't have a rom loaded
    
    Core::GameBoy* gb = new Core::GameBoy(appState.IsBootRomEnabled);

    if (argc >= 2)
    {
        const char* filePath = argv[1];
        gb->m_CPU.m_EnableLogging = true;
        gb->LoadRom(filePath);
        gb->Clock(1000 * 60); // run for 2 seconds
        isRunning = false;
    }
    
    Window*  window = new Window(1280, 720, "DotMatrixBoy", gb, appState);
    window->Initialize();
    
    // Widgets
    window->AttachWidget(std::make_unique<FileDialog>(gb, appState));
    window->AttachWidget(std::make_unique<MenuBar>(gb, appState));
    window->AttachWidget(std::make_unique<LCD>(gb->m_PPU.GetLCDPixels(), window->GetRenderer()));
    window->AttachWidget(std::make_unique<AudioDebugger>(gb));
    window->AttachWidget(std::make_unique<Debugger>(gb));
    window->AttachWidget(std::make_unique<MemoryMap>(gb));
    window->AttachWidget(std::make_unique<VRAMViewer>(gb, window->GetRenderer()));
    window->AttachWidget(std::make_unique<Console>());
    window->AttachWidget(std::make_unique<RomInfo>(gb));

    // Main loop
    while (isRunning)
    {
        appState.IsPaused = gb->IsPaused();
        gb->SetBootRomEnabled(appState.IsBootRomEnabled);

        window->Update(isRunning);

        gb->Clock((float)(std::min)((int)window->GetElapsedTime(), 20));

        window->BeginRender();

        #ifdef DEBUG
        bool show_demo_window = true;
        ImGui::ShowDemoWindow(&show_demo_window);
        #endif

        // render widgets
        for (auto& widget : window->GetWidgets())
        {
            widget->Render();
        }

        window->EndRender();

        if (window->ShouldExit())
        {
            isRunning = false;
        }
    }

    appState.SaveStateToFile();

    // clean up
    delete gb;
    delete window;

    _CrtDumpMemoryLeaks();
}
