
#include "GameBoy.h"
#include "Cpu.h"
#include "Cartridge.h"
#include "Defines.h"

#include <fstream>
#include <vector>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

class DotMatrixBoy : public olc::PixelGameEngine
{
public:
	DotMatrixBoy() { sAppName = "DotMatrixBoy"; }

	GameBoy gb;
	std::shared_ptr<Cartridge> cart;
	bool isPaused = false;

	std::string FormatInt(uint32_t n, uint8_t d)
	{
		std::stringstream s;
		s << n;
		return s.str();
	}

	std::string FormatHex(uint32_t n, uint8_t d)
	{
		std::string s(d, '0');
		for (int i = d - 1; i >= 0; i--, n >>= 4)
			s[i] = "0123456789ABCDEF"[n & 0xF];
		return s;
	};

	void DrawCpu(int x, int y)
	{
		DrawString(x, y, "CPU STATUS:", olc::WHITE);
		DrawString(x, y + 10, "Cycles: " + FormatInt(gb.cpu.m_TotalCycles, 1));
		DrawString(x, y + 20, "SP: $" + FormatHex(gb.cpu.State.SP, 4));
		DrawString(x, y + 30, "PC: $" + FormatHex(gb.cpu.State.PC, 4));
		DrawString(x, y + 40, "AF: $" + FormatHex(gb.cpu.State.AF, 4));
		DrawString(x, y + 50, "BC: $" + FormatHex(gb.cpu.State.BC, 4));
		DrawString(x, y + 60, "DE: $" + FormatHex(gb.cpu.State.DE, 4));
		DrawString(x, y + 70, "HL: $" + FormatHex(gb.cpu.State.HL, 4));
		DrawString(x, y + 80, "IE: " + FormatInt(gb.ReadFromMemoryMap(HW_INTERRUPT_ENABLE), 1));
	}

	void DrawRam(int x, int y, uint16_t nAddr, int nRows, int nColumns)
	{
		DrawString(x, y, "RAM:", olc::WHITE);
		int nRamX = x, nRamY = y + 10;
		for (int row = 0; row < nRows; row++)
		{
			std::string sOffset = "$" + FormatHex(nAddr, 4) + ":";
			for (int col = 0; col < nColumns; col++)
			{
				sOffset += " " + FormatHex(gb.ReadFromMemoryMap(nAddr), 2);
				nAddr += 1;
			}
			DrawString(nRamX, nRamY, sOffset);
			nRamY += 10;
		}
	}

	void DrawCharacterRam(int x, int y)
	{
		DrawString(x, y, "CHARACTER RAM:", olc::WHITE);

		y += 10;

		int count = 0;
		for (uint16_t byte = 0x8000; byte <= 0x97FF; byte += 2 )
		{
			uint8_t firstByte = gb.ReadFromMemoryMap(byte);
			uint8_t secondByte = gb.ReadFromMemoryMap(byte + 1);
			for (int iBit = 0; iBit < 8; iBit++)
			{
				uint8_t firstBit = (firstByte >> iBit) & 0x01;
				uint8_t secondBit = (firstByte >> iBit) & 0x01;
				int color = (secondBit << 1) | firstBit;

				int draw_x = x - iBit; // Adjusted x coordinate for drawing
				int draw_y = y + count; // Adjusted y coordinate for drawing

				if (color == 0)
					Draw(draw_x, draw_y, olc::Pixel(155,188,15));

				if (color == 1)
					Draw(draw_x, draw_y, olc::Pixel(139, 172, 15));

				if (color == 2)
					Draw(draw_x, draw_y, olc::Pixel(48, 98, 48));

				if (color == 3)
					Draw(draw_x, draw_y, olc::Pixel(15, 56, 15));
			}
			count++;

			if ((count % 128) == 0)
			{
				x -= 120;
			}
			else if ((count % 8) == 0)
			{
				x += 8; // Move x coordinate right by 8
				y -= 8;
			}
		}
	}

	void DrawLCDScreen(int x, int y)
	{
		DrawString(x, y, "LCD", olc::WHITE);

		y += 10;

		for ( int i= 1; i <= 23040; i++ )
		{
			uint8_t pixel = gb.ppu.m_lcdPixels[i];

			if (pixel == 0)
				Draw(x, y, olc::Pixel(155, 188, 15));

			if (pixel == 1)
				Draw(x, y, olc::Pixel(139, 172, 15));

			if (pixel == 2)
				Draw(x, y, olc::Pixel(48, 98, 48));

			if (pixel == 3)
				Draw(x, y, olc::Pixel(15, 56, 15));

			if (i > 0 && (i % 160) == 0)
			{
				x -= 159;
				y++;
			}
			else
			{
				x++;
			}
		}
	}

	void DrawPPUStats(int x, int y)
	{
		DrawString(x, y, "PPU STATUS:", olc::WHITE);

		if (gb.ppu.m_CurrentMode == MODE_0_HBLANK  ) DrawString(x, y + 10, "Mode: MODE_0_HBLANK");
		if (gb.ppu.m_CurrentMode == MODE_1_VBLANK  ) DrawString(x, y + 10, "Mode: MODE_1_VBLANK");
		if (gb.ppu.m_CurrentMode == MODE_2_OAMSCAN ) DrawString(x, y + 10, "Mode: MODE_2_OAMSCAN");
		if (gb.ppu.m_CurrentMode == MODE_3_DRAWING ) DrawString(x, y + 10, "Mode: MODE_3_DRAWING");

		DrawString(x, y + 20, "Scanline: " + FormatInt(gb.ppu.m_CurrentScanLine, 1));
		DrawString(x, y + 30, "Dots This Frame: " + FormatInt(gb.ppu.m_TotalDotsThisFrame, 1));
		DrawString(x, y + 40, "Total Frames: " + FormatInt(gb.ppu.m_TotalFrames, 1));
	}

	bool OnUserCreate()
	{
		cart = std::make_shared<Cartridge>("../hello-world.gb");

		gb.InsertCartridge(*cart);
		gb.Run();

		return true;
	}

	bool OnUserUpdate(float fElapsedTime)
	{
		//// Wait for the remaining time to maintain the desired clock speed
		//if (fElapsedTime < INSTRUCTION_TIME_SEC) {
		//	std::this_thread::sleep_for(std::chrono::duration<double>(INSTRUCTION_TIME_SEC - fElapsedTime));
		//}

		Clear(olc::BLACK);

		if (GetKey(olc::Key::SPACE).bPressed)
		{
			gb.Clock();
		}

		if (GetKey(olc::Key::P).bPressed)
		{
			isPaused = !isPaused;
		}

		if (!isPaused)
		{
			gb.Clock();
		}

		DrawCpu(10, 10);
		DrawRam(160, 110, HW_STAT_LCD_STATUS, 5, 16);
		DrawCharacterRam(10, 110);
		DrawPPUStats(160, 10);
		DrawLCDScreen(160, 200);
		return true;
	}
};

int main()
{
	DotMatrixBoy emu;
	emu.Construct(680, 480, 2, 2);
	emu.Start();
	return 0;
}