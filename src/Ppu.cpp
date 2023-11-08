
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
	// The values of LYC and LY are compared. When both values are identical, 
	// the “LYC=LY” flag in the STAT register is set, and (if enabled) a 
	// STAT interrupt is requested.
	uint8_t ly = gb.ReadFromMemoryMap(HW_LY_LCD_Y_COORD);
	uint8_t lyc = gb.ReadFromMemoryMap(HW_LYC_LY_COMPARE);
	
	uint8_t stat = gb.ReadFromMemoryMap(HW_STAT_LCD_STATUS);

	if (ly == lyc)
		setFlag(stat, STAT_LYC_EQUAL_LY);
	else
		clearFlag(stat, STAT_LYC_EQUAL_LY);

	gb.WriteToMemoryMap(HW_STAT_LCD_STATUS, stat);

	m_TotalDotsThisFrame += 4;

	int dotsThisScanline = m_TotalDotsThisFrame % 456;

	if (m_CurrentScanLine >= 145 && m_CurrentScanLine <= 153)
	{
		m_CurrentMode = MODE_1_VBLANK;
	}
	else if (dotsThisScanline >= 0 && dotsThisScanline <= 80 )
	{ 
		m_CurrentMode = MODE_2_OAMSCAN;
	}
	else if (dotsThisScanline >= 204)
	{
		m_CurrentMode = MODE_0_HBLANK;
	}
	else if (dotsThisScanline >= 81)
	{
		m_CurrentMode = MODE_3_DRAWING; 
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

void Ppu::UpdateFIFO(pixelFIFO& fifo, uint8_t ly, uint8_t scy )
{
	switch (fifo.fifo_state)
	{
		case FIFO_GET_TILE:
		{
			(ly + scy) / 8;
		}
	}
}