
#include "GameBoy.h"
#include "Ppu.h"

Ppu::Ppu() {}
Ppu::~Ppu() {}

void Ppu::Clock(GameBoy& gb)
{
	/*if (!gb.ReadFromMemoryMapRegister(HW_LCDC_LCD_CONTROL, LCDC_Flags::LCDC_LCD_PPU_ENABLE))
	{
		return;
	}*/

	LCD_Mode currentMode = readLCDMode(gb);
	m_CycleCount++;

	switch (currentMode)
	{
		case LCD_Mode::MODE_2_OAMSCAN:
			processOAM(gb);
			break;
		case LCD_Mode::MODE_3_DRAWING:
			processDrawing(gb);
			break;
		case LCD_Mode::MODE_0_HBLANK:
			processHBlank(gb);
			break;
		case LCD_Mode::MODE_1_VBLANK:
			processVBlank(gb);
			break;
	}
}

LCD_Mode Ppu::GetMode(GameBoy& gb)
{
	return readLCDMode(gb);
}

void Ppu::processOAM(GameBoy& gb)
{
	if (m_CycleCount >= OAM_CYCLES)
	{
		writeLCDMode(gb, LCD_Mode::MODE_3_DRAWING);
		m_CycleCount -= OAM_CYCLES;
	}

	// TODO: need to handle OAM scanning
}

void Ppu::processDrawing(GameBoy& gb)
{
	if (m_CycleCount >= DRAWING_CYCLES)
	{
		writeLCDMode(gb, LCD_Mode::MODE_0_HBLANK);
		m_CycleCount -= DRAWING_CYCLES;
	}
}

void Ppu::processHBlank(GameBoy& gb)
{
	if (m_CycleCount >= HBLANK_CYCLES)
	{
		drawBGToBuffer(gb);

		uint8_t ly = gb.ReadFromMemoryMap(HW_LY_LCD_Y_COORD);
		if (ly == 143)
			writeLCDMode(gb, LCD_Mode::MODE_1_VBLANK);
		else
			writeLCDMode(gb, LCD_Mode::MODE_2_OAMSCAN);

		gb.WriteToMemoryMap(HW_LY_LCD_Y_COORD, ly + 1);
		m_CycleCount -= HBLANK_CYCLES;
	}
}

void Ppu::processVBlank(GameBoy& gb)
{
	if (m_CycleCount >= VBLANK_CYCLES)
	{
		uint8_t ly = gb.ReadFromMemoryMap(HW_LY_LCD_Y_COORD);
		
		if (ly == 144)
		{
			clearBuffer();
		}
		
		if (ly == 153)
		{
			writeLCDMode(gb, LCD_Mode::MODE_2_OAMSCAN);
			gb.WriteToMemoryMap(HW_LY_LCD_Y_COORD, 0);
		}
		else
			gb.WriteToMemoryMap(HW_LY_LCD_Y_COORD, ly + 1);

		m_CycleCount -= VBLANK_CYCLES;
	}
}

// This will draw one line of the background to the buffer 
void Ppu::drawBGToBuffer(GameBoy& gb)
{
	//if (!gb.ReadFromMemoryMapRegister(HW_LCDC_LCD_CONTROL, LCDC_Flags::LCDC_BG_WINDOW_ENABLE_PRIORITY))
	//{
	//	return;
	//}

	uint8_t lcdY = gb.ReadFromMemoryMap(HW_LY_LCD_Y_COORD);
	uint8_t scrollY = gb.ReadFromMemoryMap(HW_SCY_VIEWPORT_Y_POS);
	uint8_t scrollX = gb.ReadFromMemoryMap(HW_SCX_VIEWPORT_X_POS);

	// Wrap the viewport around Y
	uint8_t startBackgroundTileY = (scrollY + lcdY) / 8 >= 32
		? (scrollY + lcdY) / 8 % 32
		: (scrollY + lcdY) / 8;
	uint8_t backgroundTileYOffset = (scrollY + lcdY) % 8;

	uint8_t startBackgroundTileX = scrollX / 8;
	uint8_t backgroundTileXOffset = scrollX % 8;

	// The screen is 20 tiles horizontal (160 pixel / 8 pixel per Tile) but will render 21 tiles to scroll correctly
	for (int i = 0; i < 21; i++)
	{
		// Wrap the viewport around X
		uint8_t tileNrX = startBackgroundTileX + i >= 32
			? (startBackgroundTileX + i) % 32
			: startBackgroundTileX + i;

		int currentTilePos = startBackgroundTileY * 32 + tileNrX;
		uint8_t tileId = gb.ReadFromMemoryMap(0x9800 + currentTilePos);

		uint16_t address;
		if (tileId < 128) {
			// Tiles 0-127 are located in the Tile Data Table at addresses 0x8800 to 0x97FF
			address = 0x9000 + (tileId * 16);  // Each tile is 16 bytes
		}
		else {
			// Tiles 128-255 are located in the Tile Data Table at addresses 0x8000 to 0x8FFF
			address = 0x8000 + ((tileId - 128) * 16);  // Each tile is 16 bytes
		}

		// need to offset the address based on the y position (backgroundTileYOffset) inside the tile
		uint8_t firstByte = gb.ReadFromMemoryMap(address + (backgroundTileYOffset * 2));
		uint8_t secondByte = gb.ReadFromMemoryMap(address + (backgroundTileYOffset * 2) + 1);

		// each tile is 8 pixels wide
		for (int j = 0; j < 8; j++)
		{
			// make sure we are in bounds
			if (i == 0 && j < backgroundTileXOffset) continue;
			if (i == 20 && j >= backgroundTileXOffset) continue;
			
			uint8_t firstBit = (firstByte >> j) & 0x01;
			uint8_t secondBit = (secondByte >> j) & 0x01;
			int color = (secondBit << 1) | firstBit;

			int x = (i * 8) - backgroundTileXOffset - 1;
			x = x + 8 - j; // fixes reversed pixels
			writeToBuffer(x, lcdY, color);
		}
	}
}

void Ppu::writeToBuffer(int x, int y, int color)
{
	m_lcdPixels[y * LCD_WIDTH + x] = color;
}

void Ppu::clearBuffer()
{
	for (int y = 0; y < LCD_HEIGHT; y++)
	{
		for (int x = 0; x < LCD_WIDTH; x++)
		{
			m_lcdPixels[y * LCD_WIDTH + x] = 0;
		}
	}
}

// The LCD mode is special because it is contained in two bits within the
//	STAT register. This seems to be the only place this happens so this 
//  function exists just to simply the process of retreiving this value
LCD_Mode Ppu::readLCDMode(GameBoy& gb)
{
	bool lowBit = gb.ReadFromMemoryMapRegister(HW_STAT_LCD_STATUS, STAT_FLags::STAT_PPU_MODE_LBIT);
	bool highBit = gb.ReadFromMemoryMapRegister(HW_STAT_LCD_STATUS, STAT_FLags::STAT_PPU_MODE_HBIT);

	uint8_t value = 0;
	value |= lowBit ? 0x01 : 0;
	value |= highBit ? 0x02 : 0;

	return static_cast<LCD_Mode>(value);
}

void Ppu::writeLCDMode(GameBoy& gb, LCD_Mode mode)
{
	uint8_t value = static_cast<uint8_t>(mode);
	gb.WriteToMemoryMapRegister(HW_STAT_LCD_STATUS, STAT_FLags::STAT_PPU_MODE_HBIT, (value >> 1) & 0x1);
	gb.WriteToMemoryMapRegister(HW_STAT_LCD_STATUS, STAT_FLags::STAT_PPU_MODE_LBIT, value & 0x1);
}
