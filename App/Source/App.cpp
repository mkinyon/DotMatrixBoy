
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

int main(int argv, char** args)
{
    sAppState appState;

    const char* romName = "../Roms/drmario.gb";// "../Roms/Mooneye/emulator-only/mbc1/bits_bank2.gb";

    std::unique_ptr<Core::Cartridge> cart = std::make_unique<Core::Cartridge>(romName, appState.IsBootRomEnabled);
    Core::GameBoy* gb = new Core::GameBoy(*cart);
    Window*  window = new Window(1280, 720, "DotMatrixBoy", gb, appState);
    window->Initialize();

    gb->Run(appState.IsBootRomEnabled);
    
    // Widgets
    FileDialog* fileDialog = new FileDialog(gb, appState);
    MenuBar* menuBar = new MenuBar(appState);
    LCD* lcdWindow = new LCD(gb->m_PPU.GetLCDPixels(), window->GetRenderer());
    AudioDebugger* audioDebugger = new AudioDebugger(gb);
    Debugger* debugger = new Debugger(gb);
    MemoryMap* memoryMap = new MemoryMap(gb);
    VRAMViewer* vramViewer = new VRAMViewer(gb, window->GetRenderer());
    Console* console = new Console();
    RomInfo* romInfo = new RomInfo(gb);

    bool show_demo_window = true;
    bool show_another_window = false;

    // Main loop
    bool isRunning = true;
    while (isRunning)
    {
        if (appState.IsPaused)
        {
            gb->Pause();
        }
        else
        {
            gb->Unpause();
        }

        window->Update(isRunning);

        gb->Clock((float)(std::min)((int)window->GetElapsedTime(), 20));

        window->BeginRender();

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // render widgets
        menuBar->Render();
        fileDialog->Render();
        lcdWindow->Render();
        debugger->Render();
        //memoryMap->Render();
        vramViewer->Render();
        audioDebugger->Render();
        console->Render();
        romInfo->Render();

        window->EndRender();

        if (menuBar->m_ExitPressed)
        {
            isRunning = false;
        }
    }

    appState.SaveStateToFile();

    // clean up
    delete menuBar;
    delete fileDialog;
    delete lcdWindow;
    delete debugger;
    delete memoryMap;
    delete vramViewer;
    delete audioDebugger;
    delete console;

    delete gb;
    delete window;

    _CrtDumpMemoryLeaks();

    return 0;
}
