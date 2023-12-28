#pragma once
#include <stdint.h>

#include "Defines.h"

namespace Core
{
	class Ppu
	{
	public:
		Ppu(GameBoy& gb);
		~Ppu();

	public:
		void Clock();
		LCD_Mode GetMode();

	public:
		GameBoy& gb;
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
		void drawSpritesToBuffer();

		void writeToBuffer(int x, int y, uint8_t bgPalette, int colorIndex);
		void copyBackBufferToLCD();
		void clearBackBuffer();

		void RefreshOAMEntries();

	private:
		uint8_t m_backBuffer[160 * 144] = {};

		struct OAM
		{
			uint16_t address = 0;
			uint8_t xPos;
			uint8_t yPos;
			uint8_t tileIndex;

			bool priority;
			bool xFlip;
			bool yFlip;
			bool paletteOneSelected;
		};

		OAM oamEntries[40] = {};
	};
}

