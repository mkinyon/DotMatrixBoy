
#include "Core\GameBoy.h"
#include "Core\Cpu.h"
#include "Core\Cartridge.h"
#include "Core\Defines.h"
#include "Core\Utils.h"

#include <vector>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <stdio.h>
#include <SDL.h>

#include "LCDWindow.h"

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

Core::GameBoy gb;
std::shared_ptr<Core::Cartridge> cart;
bool isPaused = true;
bool enableBootRom = false;
const char* romName = "../Roms/hello-world.gb";
//const char* romName = "../Roms/02-interrupts.gb";
//const char* romName = "../Roms/tetris.gb";

// Main code
int main(int, char**)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with SDL_Renderer graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("DotMatrixBoy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return 0;
    }

    cart = std::make_shared<Core::Cartridge>(romName, enableBootRom);
    gb.InsertCartridge(*cart);
    gb.Run(enableBootRom);
    App::LCDWindow lcdWindow(renderer);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // enable docking

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        gb.Clock();

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        lcdWindow.Render(gb.ppu.m_lcdPixels);

        // Rendering
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}




//#define OLC_PGE_APPLICATION
//#include "olcPixelGameEngine.h"

//class DotMatrixBoy : public olc::PixelGameEngine
//{
//public:
//	DotMatrixBoy() { sAppName = "DotMatrixBoy"; }
//
//private:
//	Core::GameBoy gb;
//	std::shared_ptr<Core::Cartridge> cart;
//	bool isPaused = true;
//	bool enableBootRom = false;
//	const char* romName = "../Roms/hello-world.gb";
//	//const char* romName = "../Roms/02-interrupts.gb";
//	//const char* romName = "../Roms/tetris.gb";
//
//	std::map<uint16_t, std::string> disasmMap;
//
//
//private:
//	void DrawCpuStats(int x, int y)
//	{
//		DrawString(x, y, "CPU STATUS:", olc::WHITE);
//		DrawString(x, y + 10, "Cycles: " + Core::FormatInt(gb.cpu.m_TotalCycles, 1));
//		DrawString(x, y + 20, "SP: $" + Core::FormatHex(gb.cpu.State.SP, 4));
//		DrawString(x, y + 30, "PC: $" + Core::FormatHex(gb.cpu.State.PC, 4));
//		DrawString(x, y + 40, "AF: $" + Core::FormatHex(gb.cpu.State.AF, 4));
//		DrawString(x, y + 50, "BC: $" + Core::FormatHex(gb.cpu.State.BC, 4));
//		DrawString(x, y + 60, "DE: $" + Core::FormatHex(gb.cpu.State.DE, 4));
//		DrawString(x, y + 70, "HL: $" + Core::FormatHex(gb.cpu.State.HL, 4));
//		DrawString(x, y + 80, "IE: " + Core::FormatInt(gb.ReadFromMemoryMap(Core::HW_INTERRUPT_ENABLE), 1));
//	}
//
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
//	void DrawLCDScreen(int x, int y)
//	{
//		DrawString(x, y, "LCD", olc::WHITE);
//
//		y += 10;
//
//		for (int i = 1; i <= LCD_WIDTH * LCD_HEIGHT; i++)
//		{
//			uint8_t pixel = gb.ppu.m_lcdPixels[i - 1];
//
//			if (pixel == 0)
//				Draw(x, y, olc::Pixel(155, 188, 15));
//
//			if (pixel == 1)
//				Draw(x, y, olc::Pixel(139, 172, 15));
//
//			if (pixel == 2)
//				Draw(x, y, olc::Pixel(48, 98, 48));
//
//			if (pixel == 3)
//				Draw(x, y, olc::Pixel(15, 56, 15));
//
//			if (i > 0 && (i % 160) == 0)
//			{
//				x -= 159;
//				y++;
//			}
//			else
//			{
//				x++;
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
//
//	void DrawDisassembly(int x, int y, int nLines)
//	{
//		auto it_a = disasmMap.find(gb.cpu.State.PC);
//		int nLineY = (nLines >> 1) * 10 + y;
//		if (it_a != disasmMap.end())
//		{
//			DrawString(x, nLineY, (*it_a).second, olc::RED);
//			while (nLineY < (nLines * 10) + y)
//			{
//				nLineY += 10;
//				if (++it_a != disasmMap.end())
//				{
//					DrawString(x, nLineY, (*it_a).second);
//				}
//			}
//		}
//
//		it_a = disasmMap.find(gb.cpu.State.PC);
//		nLineY = (nLines >> 1) * 10 + y;
//		if (it_a != disasmMap.end())
//		{
//			while (nLineY > y)
//			{
//				nLineY -= 10;
//				if (--it_a != disasmMap.end())
//				{
//					DrawString(x, nLineY, (*it_a).second);
//				}
//			}
//		}
//	}
//
//	bool OnUserCreate()
//	{
//		cart = std::make_shared<Core::Cartridge>(romName, enableBootRom);
//
//		gb.InsertCartridge(*cart);
//		gb.Run(enableBootRom);
//
//		disasmMap = gb.cpu.DisassebleAll(gb);
//
//		return true;
//	}
//
//	bool OnUserUpdate(float fElapsedTime)
//	{
//		Clear(olc::BLACK);
//
//		if (GetKey(olc::Key::S).bPressed)
//		{
//			do { gb.Clock(); } while (!gb.cpu.m_InstructionCompleted);
//		}
//
//		if (GetKey(olc::Key::SPACE).bHeld && GetKey(olc::Key::SHIFT).bHeld)
//		{
//			for (int i = 0; i < 1000; i++)
//			{
//				do { gb.Clock(); } while (!gb.cpu.m_InstructionCompleted);
//			}
//		}
//		else if (GetKey(olc::Key::SPACE).bHeld)
//		{
//			do { gb.Clock(); } while (!gb.cpu.m_InstructionCompleted);
//		}
//
//		if (GetKey(olc::Key::P).bPressed)
//		{
//			isPaused = !isPaused;
//		}
//
//		if (!isPaused)
//		{
//			float elapsedTimeInMS = fElapsedTime * 1000.0f;
//			float cyclesToRun = elapsedTimeInMS * Core::CYCLES_PER_MS;
//
//			for (int i = 0; i < cyclesToRun; i++)
//			{
//				gb.Clock();
//			}
//		}
//
//		DrawCpuStats(10, 10);
//		DrawPPUStats(10, 110);
//		DrawRam(200, 10, 0xCEEE, 20, 16);
//		DrawCharacterRam(370, 240);
//		DrawLCDScreen(500, 240);
//		DrawDisassembly(10, 240, 20);
//
//		return true;
//	}
//};
//
//DotMatrixBoy emu;