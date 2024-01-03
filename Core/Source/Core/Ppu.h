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

	public:
		Mmu& mmu;
		unsigned int m_CycleCount = 0; // each cycle equates to one dot (pixel)
		unsigned int m_TotalFrames = 0;
		unsigned int m_CurrentScanLine = 0;
		unsigned int m_TotalDotsThisFrame = 0;

		uint8_t m_lcdPixels[160 * 144] = {};
		

	private:
		// lcd mode functions
		void processOAM();
		void processDrawing();
		void processHBlank();
		void processVBlank();
		LCD_Mode readLCDMode();
		void writeLCDMode(LCD_Mode mode);

		// drawing functions
		void drawBGToBuffer();
		void drawWindowToBuffer();
		void drawOAMToBuffer();

		uint8_t ReadFromBuffer(int x, int y);
		void writeToBuffer(int x, int y, uint8_t bgPalette, int colorIndex);
		void copyBackBufferToLCD();
		void clearBackBuffer();

		void RefreshOAMEntries();
		uint16_t GetTileAddressFromTileId(uint8_t tileId);

	private:
		uint8_t m_backBuffer[160 * 144] = {};
		OAM oamEntries[40] = {};
	};
}

