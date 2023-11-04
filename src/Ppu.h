#pragma once
#include <stdint.h>

#include "Defines.h"

class Ppu
{
public:
	Ppu();
	~Ppu();

public:
	void Clock(GameBoy& gb);

public:
	LCD_Mode m_CurrentMode;
	unsigned int m_TotalFrames = 0;
	unsigned int m_CurrentScanLine = 0;
	unsigned int m_TotalDotsThisFrame = 0;

	uint8_t lcdPixels[160 * 144];

private:
	typedef struct 
	{
		uint8_t pixel; // 2 bit color
		uint8_t palette; // 0 or 1
		uint8_t priority;
	} pixelFIFO_Item;

	typedef struct 
	{
		pixelFIFO_Item fifo[8];
		uint8_t read_end;
		uint8_t size;
	} pixelFIFO;

	pixelFIFO bgFIFO;
	pixelFIFO oamFIFO;
};

