#pragma once
#include <stdint.h>

#include "Mmu.h"
#include "Defines.h"

namespace Core
{
	class Ppu
	{
	public:
		Ppu(Mmu& mmu);
		~Ppu();

	public:
		struct OAM
		{
			uint16_t address = 0;
			uint8_t xPos = 0;
			uint8_t yPos = 0;
			uint8_t tileIndex = 0;

			bool bgPriority = false;
			bool xFlip = false;
			bool yFlip = false;
			bool paletteOneSelected = false;
			bool isTall = false;

			bool operator < (const OAM& other) const
			{
				return (xPos < other.xPos) || (xPos == other.xPos && address < other.address);
			}
		};

		void Clock();
		LCD_Mode GetMode();
		OAM* GetOAMEntries();

		int GetTotalFrames();
		int GetCurrentScanLine();
		int GetTotalDotsThisFrame();
		uint8_t* GetLCDPixels();

	public:
		Mmu& m_MMU;
		

	private:
		// lcd mode functions
		void ProcessOAM();
		void ProcessDrawing();
		void ProcessHBlank();
		void ProcessVBlank();
		LCD_Mode ReadLCDMode();
		void WriteLCDMode(LCD_Mode mode);

		// drawing functions
		void DrawBGToBuffer();
		void DrawWindowToBuffer();
		void DrawOAMToBuffer();

		uint8_t ReadFromBuffer(int x, int y);
		void WriteToBuffer(int x, int y, uint8_t bgPalette, int colorIndex);
		void CopyBackBufferToLCD();
		void ClearBackBuffer();

		void RefreshOAMEntries();
		uint16_t GetTileAddressFromTileId(uint8_t tileId);

		void ProcessLYC();

	private:
		uint8_t m_BackBuffer[160 * 144] = {};
		OAM m_OAMEntries[40] = {};
		LCD_Mode m_LCDMode = LCD_Mode::MODE_0_HBLANK;

		unsigned int m_CycleCount = 0; // each cycle equates to one dot (pixel)
		unsigned int m_TotalFrames = 0;
		unsigned int m_CurrentScanLine = 0;
		unsigned int m_TotalDotsThisFrame = 0;

		uint8_t m_LCDPixels[160 * 144] = {};
	};
}

