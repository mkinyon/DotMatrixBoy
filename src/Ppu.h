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
	LCD_Mode GetMode(GameBoy& gb);

public:
	unsigned int m_CycleCount = 0; // each cycle equates to one dot (pixel)
	unsigned int m_TotalFrames = 0;
	unsigned int m_CurrentScanLine = 0;
	unsigned int m_TotalDotsThisFrame = 0;

	uint8_t m_lcdPixels[160 * 144];

private:
	typedef enum
	{
		FIFO_GET_TILE,
		FIFO_GET_TILE_LOW,
		FIFO_GET_TILE_HIGH,
		FIFO_SLEEP,
		FIFO_PUSH
	} FIFO_State;

	//typedef struct 
	//{
	//	uint8_t pixel; // 2 bit color
	//	uint8_t palette; // 0 or 1
	//	uint8_t priority;
	//} pixelFIFO_Item;

	//typedef struct 
	//{
	//	pixelFIFO_Item fifo[8];
	//	uint8_t read_end;
	//	uint8_t size;
	//	FIFO_State fifo_state;
	//} pixelFIFO;

	// lcd mode functions
	void processOAM(GameBoy& gb);
	void processDrawing(GameBoy& gb);
	void processHBlank(GameBoy& gb);
	void processVBlank(GameBoy& gb);
	LCD_Mode readLCDMode(GameBoy& gb);
	void writeLCDMode(GameBoy& gb, LCD_Mode mode);

	// drawing functions
	void drawBGToBuffer(GameBoy& gb);

	//void enqueueFIFO(pixelFIFO* fifo, pixelFIFO_Item item);
	//pixelFIFO_Item dequeueFIFO(pixelFIFO* fifo);
	//void clearFIFO(pixelFIFO* fifo);

	//void pixelFetcher(pixelFIFO& fifo, uint8_t scx, uint8_t scy);

	//pixelFIFO bgFIFO;
	//pixelFIFO oamFIFO;
};

