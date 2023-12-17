#pragma once
#include <stdint.h>

#include "Defines.h"

namespace Core
{
	class Ppu
	{
	public:
		Ppu();
		~Ppu();

	public:
		void Clock(GameBoy& gb);
		LCD_Mode GetMode(GameBoy& gb);

	public:
		unsigned int m_CycleCount = 0; // each cycle equates to one dot (pixel)
		unsigned int m_TotalFrames = 0;
		unsigned int m_CurrentScanLine = 0;
		unsigned int m_TotalDotsThisFrame = 0;

		uint8_t m_lcdPixels[160 * 144];
		uint8_t m_backBuffer[160 * 144];

	private:
		// lcd mode functions
		void processOAM(GameBoy& gb);
		void processDrawing(GameBoy& gb);
		void processHBlank(GameBoy& gb);
		void processVBlank(GameBoy& gb);
		LCD_Mode readLCDMode(GameBoy& gb);
		void writeLCDMode(GameBoy& gb, LCD_Mode mode);

		// drawing functions
		void drawBGToBuffer(GameBoy& gb);

		void writeToBuffer(int x, int y, uint8_t bgPalette, int colorIndex);
		void copyBackBufferToLCD();
		void clearBackBuffer();
	};
}

