
#include "Core\GameBoy.h"
#include "Core\Cpu.h"
#include "Core\Cartridge.h"
#include "Core\Defines.h"
#include "Core\Utils.h"

#include <vector>

#include "imgui.h"
#include "imgui_impl_vulkan.cpp"
#include "imgui_impl_glfw.cpp"

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

int main()
{
	/*emu.Construct(680, 480, 2, 2);
	emu.Start();*/
	return 0;
}