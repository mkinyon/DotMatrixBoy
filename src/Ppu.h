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

	uint8_t m_lcdPixels[160 * 144];

private:
	typedef struct 
	{
		uint8_t pixel; // 2 bit color
		uint8_t palette; // 0 or 1
		uint8_t priority;
	} pixelFIFO_Item;

	typedef enum
	{
		FIFO_GET_TILE,
		FIFO_GET_TILE_LOW,
		FIFO_GET_TILE_HIGH,
		FIFO_SLEEP,
		FIFO_PUSH
	} FIFO_State;

	typedef struct 
	{
		pixelFIFO_Item fifo[8];
		uint8_t read_end;
		uint8_t size;
		FIFO_State fifo_state;
	} pixelFIFO;

	void UpdateFIFO(pixelFIFO& fifo, uint8_t ly, uint8_t scy);

	pixelFIFO bgFIFO;
	pixelFIFO oamFIFO;
};

