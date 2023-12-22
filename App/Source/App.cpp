
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
#include "UI/LCD.h"
#include "UI/Debugger.h"
#include "UI/MemoryMap.h"

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

App::Window window(1280, 720, "DotMatrixBoy");

Core::GameBoy gb;
std::shared_ptr<Core::Cartridge> cart;
bool isPaused = true;
bool enableBootRom = true;
const char* romName = "../Roms/hello-world.gb";
//const char* romName = "../Roms/02-interrupts.gb";
//const char* romName = "../Roms/tetris.gb";

// Main code
int main(int, char**)
{
    window.Initialize();

    cart = std::make_shared<Core::Cartridge>(romName, enableBootRom);
    gb.InsertCartridge(*cart);
    gb.Run(enableBootRom);
    
    // Widgets
    App::LCD lcdWindow(gb.ppu.m_lcdPixels, window.GetRenderer());
    App::Debugger debugger(gb);
    App::MemoryMap memoryMap(gb);

    bool show_demo_window = true;
    bool show_another_window = false;

    // Main loop
    bool done = false;
    while (!done)
    {
        window.Update(done);

        gb.Clock((float)std::min((int)window.GetElapsedTime(), 16 ));
        //gb.Clock(16);

        window.BeginRender();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // render widgets
        lcdWindow.Render();
        debugger.Render();
        memoryMap.Render();

        window.EndRender();
    }

    return 0;
}





//	void DrawRam(int x, int y, uint16_t nAddr, int nRows, int nColumns)
//	{
//		DrawString(x, y, "RAM:", olc::WHITE);
//		int nRamX = x, nRamY = y + 10;
//		for (int row = 0; row < nRows; row++)
//		{
//			std::string sOffset = "$" + Core::FormatHex(nAddr, 4) + ":";
//			for (int col = 0; col < nColumns; col++)
//			{
//				sOffset += " " + Core::FormatHex(gb.ReadFromMemoryMap(nAddr), 2);
//				nAddr += 1;
//			}
//			DrawString(nRamX, nRamY, sOffset);
//			nRamY += 10;
//		}
//	}
//
//	void DrawCharacterRam(int x, int y)
//	{
//		DrawString(x, y, "CHARACTER RAM:", olc::WHITE);
//
//		y += 10;
//
//		int count = 0;
//		for (uint16_t byte = 0x8000; byte <= 0x97FF; byte += 2)
//		{
//			uint8_t firstByte = gb.ReadFromMemoryMap(byte);
//			uint8_t secondByte = gb.ReadFromMemoryMap(byte + 1);
//			for (int iBit = 0; iBit < 8; iBit++)
//			{
//				uint8_t firstBit = (firstByte >> iBit) & 0x01;
//				uint8_t secondBit = (secondByte >> iBit) & 0x01;
//				int colorIndex = (secondBit << 1) | firstBit;
//
//				int draw_x = x - iBit; // Adjusted x coordinate for drawing
//				int draw_y = y + count; // Adjusted y coordinate for drawing
//
//				// get the background palette
//				uint8_t bgPalette = gb.ReadFromMemoryMap(Core::HW_BGP_BG_PALETTE_DATA);
//				uint8_t color = bgPalette >> (colorIndex * 2) & 0x03;
//
//				if (color == 0)
//					Draw(draw_x, draw_y, olc::Pixel(155, 188, 15));
//
//				if (color == 1)
//					Draw(draw_x, draw_y, olc::Pixel(139, 172, 15));
//
//				if (color == 2)
//					Draw(draw_x, draw_y, olc::Pixel(48, 98, 48));
//
//				if (color == 3)
//					Draw(draw_x, draw_y, olc::Pixel(15, 56, 15));
//			}
//			count++;
//
//			if ((count % 128) == 0)
//			{
//				x -= 120;
//			}
//			else if ((count % 8) == 0)
//			{
//				x += 8; // Move x coordinate right by 8
//				y -= 8;
//			}
//		}
//	}
//
//	void DrawPPUStats(int x, int y)
//	{
//		DrawString(x, y, "PPU STATUS:", olc::WHITE);
//
//		if (gb.ppu.GetMode(gb) == Core::MODE_0_HBLANK) DrawString(x, y + 10, "Mode: MODE_0_HBLANK");
//		if (gb.ppu.GetMode(gb) == Core::MODE_1_VBLANK) DrawString(x, y + 10, "Mode: MODE_1_VBLANK");
//		if (gb.ppu.GetMode(gb) == Core::MODE_2_OAMSCAN) DrawString(x, y + 10, "Mode: MODE_2_OAMSCAN");
//		if (gb.ppu.GetMode(gb) == Core::MODE_3_DRAWING) DrawString(x, y + 10, "Mode: MODE_3_DRAWING");
//
//		DrawString(x, y + 20, "LCDC: " + Core::FormatInt(gb.ReadFromMemoryMap(Core::HW_LCDC_LCD_CONTROL), 1));
//		DrawString(x, y + 30, "STAT: " + Core::FormatInt(gb.ReadFromMemoryMap(Core::HW_STAT_LCD_STATUS), 1));
//
//		DrawString(x, y + 40, "SCX: " + Core::FormatInt(gb.ReadFromMemoryMap(Core::HW_SCX_VIEWPORT_X_POS), 1));
//		DrawString(x, y + 50, "SCY: " + Core::FormatInt(gb.ReadFromMemoryMap(Core::HW_SCY_VIEWPORT_Y_POS), 1));
//
//		DrawString(x, y + 60, "LY (Scanline): " + Core::FormatInt(gb.ReadFromMemoryMap(Core::HW_LY_LCD_Y_COORD), 1));
//		DrawString(x, y + 70, "LYC: " + Core::FormatInt(gb.ReadFromMemoryMap(Core::HW_LYC_LY_COMPARE), 1));
//
//		DrawString(x, y + 80, "WX: " + Core::FormatInt(gb.ReadFromMemoryMap(Core::HW_WX_WINDOW_X_POS), 1));
//		DrawString(x, y + 90, "WY: " + Core::FormatInt(gb.ReadFromMemoryMap(Core::HW_WY_WINDOW_Y_POS), 1));
//
//		DrawString(x, y + 100, "Dots This Frame: " + Core::FormatInt(gb.ppu.m_TotalDotsThisFrame, 1));
//		DrawString(x, y + 110, "Total Frames: " + Core::FormatInt(gb.ppu.m_TotalFrames, 1));
//	}
