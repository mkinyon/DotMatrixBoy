
#include "GameBoy.h"
#include "Cpu.h"
#include "Cartridge.h"

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
		DrawString(x, y + 10, "Cycles: " + FormatInt(gb.cpu.TotalCycles, 1));
		DrawString(x, y + 20, "SP: $" + FormatHex(gb.cpu.State.SP, 4));
		DrawString(x, y + 30, "PC: $" + FormatHex(gb.cpu.State.PC, 4));
		DrawString(x, y + 40, "AF: $" + FormatHex(gb.cpu.State.AF, 4));
		DrawString(x, y + 50, "BC: $" + FormatHex(gb.cpu.State.BC, 4));
		DrawString(x, y + 60, "DE: $" + FormatHex(gb.cpu.State.DE, 4));
		DrawString(x, y + 70, "HL: $" + FormatHex(gb.cpu.State.HL, 4));
	}

	void DrawCharacterRam(int x, int y, uint16_t nAddr, int nRows, int nColumns)
	{
		DrawString(x, y, "CHARACTER RAM:", olc::WHITE);
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

	bool OnUserCreate()
	{
		cart = std::make_shared<Cartridge>("../hello-world.gb");

		gb.InsertCartridge(*cart);
		gb.Run();

		return true;
	}

	bool OnUserUpdate(float fElapsedTime)
	{
		Clear(olc::BLACK);

		if (GetKey(olc::Key::SPACE).bPressed)
		{
			gb.cpu.Clock(gb);
		}

		if (GetKey(olc::Key::P).bPressed)
		{
			isPaused = !isPaused;
		}

		if (!isPaused)
		{
			gb.cpu.Clock(gb);
		}

		DrawCpu(10, 10);
		DrawCharacterRam(120, 10, 0x8000, 32, 16);
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