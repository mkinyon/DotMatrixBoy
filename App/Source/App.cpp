
#include "Window.h"

#include "Core\Cartridge.h"
#include "Core\GameBoy.h"

#include "UI/AudioDebugger.h"
#include "UI/Console.h"
#include "UI/Debugger.h"
#include "UI/FileDialog.h"
#include "UI/LCD.h"
#include "UI/MemoryMap.h"
#include "UI/MenuBar.h"
#include "UI/VRAMViewer.h"

App::Window* window;
Core::GameBoy* gb;
Core::Cartridge* cart;
bool isPaused = true;
bool enableBootRom = false;
const char* romName = "../Roms/dmg-acid2.gb";



// Main code
int main(int argv, char** args)
{
    cart = new Core::Cartridge(romName, enableBootRom);
    gb = new Core::GameBoy(*cart);
    window = new App::Window(1280, 720, "DotMatrixBoy", gb);
    window->Initialize();

    gb->Run(enableBootRom);
    
    // Widgets
    App::FileDialog* fileDialog = new App::FileDialog(gb);
    App::MenuBar* menuBar = new App::MenuBar();
    App::LCD* lcdWindow = new App::LCD(gb->ppu.m_lcdPixels, window->GetRenderer());
    App::AudioDebugger* audioDebugger = new App::AudioDebugger(gb);
    App::Debugger* debugger = new App::Debugger(gb);
    App::MemoryMap* memoryMap = new App::MemoryMap(gb);
    App::VRAMViewer* vramViewer = new App::VRAMViewer(gb, window->GetRenderer());
    App::Console* console = new App::Console();

    bool show_demo_window = true;
    bool show_another_window = false;

    // Main loop
    bool isRunning = true;
    while (isRunning)
    {
        window->Update(isRunning);

        gb->Clock((float)std::min((int)window->GetElapsedTime(), 16 ));

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

        window->EndRender();

        if (menuBar->ExitPressed)
        {
            isRunning = false;
        }
    }

    return 0;
}
