
#include "GameBoy.h"
#include "Ppu.h"

Ppu::Ppu()
{
	Ppu::m_CurrentMode = MODE_2_OAMSCAN;
}

Ppu::~Ppu() {}

void Ppu::Clock(GameBoy& gb)
{
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
		gb.WriteToMemoryMap(LY_LCD_Y_COORD, m_CurrentScanLine);
	}	

	if (m_TotalDotsThisFrame == 70224)
	{
		m_TotalFrames += 1;
		m_TotalDotsThisFrame = 0;
		m_CurrentScanLine = 0;
	}
}

void Ppu::UpdateFIFO(pixelFIFO& fifo)
{
	switch (fifo.fifo_state)
	{
		case FIFO_GET_TILE:
		{

		}
	}
}