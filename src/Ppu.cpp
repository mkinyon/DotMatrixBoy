
#include "GameBoy.h"
#include "Ppu.h"
#include "Utils.h"

Ppu::Ppu()
{
	Ppu::m_CurrentMode = MODE_2_OAMSCAN;
}

Ppu::~Ppu() {}

void Ppu::Clock(GameBoy& gb)
{
	// this is just a silly test
	//pixelFIFO_Item item;
	//item.pixel = m_TotalDotsThisFrame;
	//item.priority = 1;
	//item.palette = 1;
	//
	//enqueueFIFO(&bgFIFO, item);

	//if (bgFIFO.size == 8)
	//{
	//	for (int i = 0; i < 3; i++)
	//	{
	//		pixelFIFO_Item testItem = dequeueFIFO(&bgFIFO);
	//	}
	//}

	uint8_t scx = gb.ReadFromMemoryMap(HW_SCX_VIEWPORT_X_POS);
	uint8_t scy = gb.ReadFromMemoryMap(HW_SCY_VIEWPORT_Y_POS);
	uint8_t ly = gb.ReadFromMemoryMap(HW_LY_LCD_Y_COORD);
	uint8_t lyc = gb.ReadFromMemoryMap(HW_LYC_LY_COMPARE);
	uint8_t stat = gb.ReadFromMemoryMap(HW_STAT_LCD_STATUS);

	// The values of LYC and LY are compared. When both values are identical, 
	// the “LYC=LY” flag in the STAT register is set, and (if enabled) a 
	// STAT interrupt is requested.
	if (ly == lyc)
		setFlag(stat, STAT_LYC_EQUAL_LY);
	else
		clearFlag(stat, STAT_LYC_EQUAL_LY);

	gb.WriteToMemoryMap(HW_STAT_LCD_STATUS, stat);

	m_TotalDotsThisFrame++;

	int dotsThisScanline = m_TotalDotsThisFrame % 456;

	if (m_CurrentScanLine >= 145 && m_CurrentScanLine <= 153)
	{
		m_CurrentMode = MODE_1_VBLANK;
	}
	else if (dotsThisScanline >= 0 && dotsThisScanline <= 80 )
	{ 
		m_CurrentMode = MODE_2_OAMSCAN;
	}
	else if (dotsThisScanline > 240)
	{
		m_CurrentMode = MODE_0_HBLANK;
	}
	else if (dotsThisScanline >= 81)
	{
		m_CurrentMode = MODE_3_DRAWING; 

		// screen test - alternating pixel pattern per frame
		int x = dotsThisScanline - 81;
		int y = m_CurrentScanLine;
		int index = y * LCD_WIDTH + x;
		static int pixel = 0;
		if (index < 23040)
		{
			m_lcdPixels[index] = pixel % 4;
			pixel++;
			pixel += m_TotalFrames;
		}

		pixelFetcher(bgFIFO, scx, scy);
		// TODO
		int test = 32 * (((ly + scy) & 0xFF) / 8);

		int tilemapIndex = (((y / 8) * 160) + x) / 8;
		int tileId = gb.ReadFromMemoryMap(0x9800 + tilemapIndex);
		tileId = tileId;
	}
	
	if (dotsThisScanline % 456 == 0)
	{
		m_CurrentScanLine += 1;
		gb.WriteToMemoryMap(HW_LY_LCD_Y_COORD, m_CurrentScanLine);
	}	

	if (m_TotalDotsThisFrame == 70224)
	{
		m_TotalFrames += 1;
		m_TotalDotsThisFrame = 0;
		m_CurrentScanLine = 0;
	}
}

void Ppu::enqueueFIFO(pixelFIFO* fifo, pixelFIFO_Item item)
{
	if ( fifo->size < 8 )
	{
		// Add item to the FIFO at the write_end
		fifo->fifo[(fifo->read_end + fifo->size) % FIFO_SIZE] = item;

		// Increment the size
		fifo->size++;
	}
	else
	{
		// TODO RESEARCH: we may need to ignore this or pause the fetcher
	}
}

Ppu::pixelFIFO_Item Ppu::dequeueFIFO(pixelFIFO* fifo)
{
	pixelFIFO_Item item = fifo->fifo[fifo->read_end];
	fifo->read_end = (fifo->read_end + 1) % FIFO_SIZE;
	fifo->size--;
	return item;
}

void Ppu::clearFIFO(pixelFIFO* fifo)
{
	fifo->read_end = 0;
	fifo->size = 0;
}

// todo
void Ppu::pixelFetcher(pixelFIFO& fifo, uint8_t scx, uint8_t scy)
{
	static int cycle = 0;
	static int x = 0;
	static int y = 0;

	switch (fifo.fifo_state)
	{
		case FIFO_GET_TILE:
		{
			int xOffset = ((scx / 8) + x) & 0x1F;
			int yOffset = (m_CurrentScanLine + scy) & 255;
			int tileId = 0;
		}
	}

	cycle++;
}