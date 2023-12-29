
#include "GameBoy.h"
#include "Ppu.h"

#include <vector>

namespace Core
{
	Ppu::Ppu(GameBoy& gb) : gb(gb) {}
	Ppu::~Ppu() {}

	void Ppu::Clock()
	{
		/*if (!gb.ReadFromMemoryMapRegister(HW_LCDC_LCD_CONTROL, LCDC_Flags::LCDC_LCD_PPU_ENABLE))
		{
			return;
		}*/

		LCD_Mode currentMode = readLCDMode();
		m_CycleCount++;

		switch (currentMode)
		{
			case LCD_Mode::MODE_2_OAMSCAN:
				processOAM();
				break;
			case LCD_Mode::MODE_3_DRAWING:
				processDrawing();
				break;
			case LCD_Mode::MODE_0_HBLANK:
				processHBlank();
				break;
			case LCD_Mode::MODE_1_VBLANK:
				processVBlank();
				break;
		}
	}

	LCD_Mode Ppu::GetMode()
	{
		return readLCDMode();
	}

	void Ppu::processOAM()
	{
		if (m_CycleCount >= OAM_CYCLES)
		{
			uint8_t lcdy = gb.ReadFromMemoryMap(HW_LY_LCD_Y_COORD);
			uint8_t lcdyc = gb.ReadFromMemoryMap(HW_LYC_LY_COMPARE);

			if (lcdy == lcdyc)
			{
				gb.WriteToMemoryMapRegister(HW_STAT_LCD_STATUS, STAT_LYC_EQUAL_LY, true);

				if (gb.ReadFromMemoryMapRegister(HW_STAT_LCD_STATUS, STATE_LYC_INT_SELECT))
				{
					gb.WriteToMemoryMapRegister(HW_IF_INTERRUPT_FLAG, IF_LCD, true);
				}
			}
			else
			{
				gb.WriteToMemoryMapRegister(HW_STAT_LCD_STATUS, STAT_LYC_EQUAL_LY, false);
			}

			writeLCDMode(LCD_Mode::MODE_3_DRAWING);
			m_CycleCount -= OAM_CYCLES;
		}
	}

	void Ppu::processDrawing()
	{
		if (m_CycleCount >= DRAWING_CYCLES)
		{
			writeLCDMode(LCD_Mode::MODE_0_HBLANK);
			m_CycleCount -= DRAWING_CYCLES;
		}
	}

	void Ppu::processHBlank()
	{
		if (m_CycleCount >= HBLANK_CYCLES)
		{
			// Check for H-BLANK Interrupt
			if (gb.ReadFromMemoryMapRegister(HW_STAT_LCD_STATUS, STAT_MODE_0_INT_SELECT))
			{
				gb.WriteToMemoryMapRegister(HW_IF_INTERRUPT_FLAG, IF_LCD, true);
			}

			// render graphics
			drawBGToBuffer();
			drawWindowToBuffer();
			drawSpritesToBuffer();

			uint8_t ly = gb.ReadFromMemoryMap(HW_LY_LCD_Y_COORD);
			if (ly == 143)
				writeLCDMode(LCD_Mode::MODE_1_VBLANK);
			else
				writeLCDMode(LCD_Mode::MODE_2_OAMSCAN);

			gb.WriteToMemoryMap(HW_LY_LCD_Y_COORD, ly + 1);
			m_CycleCount -= HBLANK_CYCLES;
		}
	}

	void Ppu::processVBlank()
	{
		if (m_CycleCount >= VBLANK_CYCLES)
		{
			uint8_t ly = gb.ReadFromMemoryMap(HW_LY_LCD_Y_COORD);

			if (ly == 144)
			{
				gb.WriteToMemoryMapRegister(HW_IF_INTERRUPT_FLAG, IF_VBLANK, true);

				copyBackBufferToLCD();
				clearBackBuffer();
			}

			if (ly == 153)
			{
				writeLCDMode(LCD_Mode::MODE_2_OAMSCAN);
				gb.WriteToMemoryMap(HW_LY_LCD_Y_COORD, 0);
			}
			else
				gb.WriteToMemoryMap(HW_LY_LCD_Y_COORD, ly + 1);

			m_CycleCount -= VBLANK_CYCLES;
		}
	}

	// This will draw one line of the background to the buffer 
	void Ppu::drawBGToBuffer()
	{
		if (!gb.ReadFromMemoryMapRegister(HW_LCDC_LCD_CONTROL, LCDC_BG_WINDOW_ENABLE_PRIORITY))
		{
			return;
		}

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

			// get tile id from current tile map
			bool bgTileMapArea = gb.ReadFromMemoryMapRegister(HW_LCDC_LCD_CONTROL, LCDC_BG_TILE_MAP);
			uint8_t tileId = gb.ReadFromMemoryMap(bgTileMapArea ? BG_MAP_1 + currentTilePos : BG_MAP_0 + currentTilePos);

			// get tile address
			uint16_t tileAddress;
			bool bgTileDataArea = gb.ReadFromMemoryMapRegister(HW_LCDC_LCD_CONTROL, LCDC_BG_AND_WINDOW_TILES);
			if (bgTileDataArea)
			{
				tileAddress = 0x8000 + (tileId * 16);
			}
			else
			{
				if (tileId >= 128)
				{
					tileAddress = 0x8800 + ((tileId - 128) * 16);
				}
				else
				{
					tileAddress = 0x9000 + (tileId * 16);
				}
			}

			// need to offset the address based on the y position (backgroundTileYOffset) inside the tile
			uint8_t firstByte = gb.ReadFromMemoryMap(tileAddress + (backgroundTileYOffset * 2));
			uint8_t secondByte = gb.ReadFromMemoryMap(tileAddress + (backgroundTileYOffset * 2) + 1);

			// get the background palette
			uint8_t bgPalette = gb.ReadFromMemoryMap(HW_BGP_BG_PALETTE_DATA);

			// each tile is 8 pixels wide
			for (int j = 0; j < 8; j++)
			{
				// make sure we are in bounds
				if (i == 0 && j < backgroundTileXOffset) continue;
				if (i == 20 && j >= backgroundTileXOffset) continue;

				uint8_t firstBit = (firstByte >> j) & 0x01;
				uint8_t secondBit = (secondByte >> j) & 0x01;
				int colorIndex = (secondBit << 1) | firstBit;

				int x = (i * 8) - backgroundTileXOffset - 1;
				x = x + 8 - j; // fixes reversed pixels
				writeToBuffer(x, lcdY, bgPalette, colorIndex);
			}
		}
	}

	void Ppu::drawWindowToBuffer()
	{
		if (!gb.ReadFromMemoryMapRegister(HW_LCDC_LCD_CONTROL, LCDC_WINDOW_ENABLE))
		{
			return;
		}

		uint8_t lcdY = gb.ReadFromMemoryMap(HW_LY_LCD_Y_COORD);
		int16_t windowY = gb.ReadFromMemoryMap(HW_WY_WINDOW_Y_POS);
		int16_t windowX = gb.ReadFromMemoryMap(HW_WX_WINDOW_X_POS) - 7;

		// we only want to render the window if the window overlaps our scanline
		if (lcdY >= windowY)
		{
			uint8_t backgroundTileYOffset = (0 + lcdY) % 8;

			for (int tileX = 0; tileX < 32; tileX++)
			{
				int currentTilePos = ((lcdY - windowY) / 8) + tileX;

				// get tile id from current tile map
				bool windowTileMapArea = gb.ReadFromMemoryMapRegister(HW_LCDC_LCD_CONTROL, LCDC_WINDOW_TILE_MAP);
				uint8_t tileId = gb.ReadFromMemoryMap(windowTileMapArea ? BG_MAP_1 + currentTilePos : BG_MAP_0 + currentTilePos);

				// get tile address
				uint16_t tileAddress;
				bool windowTileDataArea = gb.ReadFromMemoryMapRegister(HW_LCDC_LCD_CONTROL, LCDC_BG_AND_WINDOW_TILES);

				if (windowTileDataArea)
				{
					tileAddress = 0x8000 + (tileId * 16);
				}
				else
				{
					if (tileId >= 128)
					{
						tileAddress = 0x8800 + ((tileId - 128) * 16);
					}
					else
					{
						tileAddress = 0x9000 + (tileId * 16);
					}
				}

				//tileAddress = 0x8AA0;

				// need to offset the address based on the y position (backgroundTileYOffset) inside the tile
				uint8_t firstByte = gb.ReadFromMemoryMap(tileAddress + (backgroundTileYOffset * 2));
				uint8_t secondByte = gb.ReadFromMemoryMap(tileAddress + (backgroundTileYOffset * 2) + 1);

				// get the background palette
				uint8_t bgPalette = gb.ReadFromMemoryMap(HW_BGP_BG_PALETTE_DATA);

				for (int j = 0; j < 8; j++)
				{
					uint8_t firstBit = (firstByte >> j) & 0x01;
					uint8_t secondBit = (secondByte >> j) & 0x01;
					int colorIndex = (secondBit << 1) | firstBit;

					int x = (tileX * 8) - j + windowX;
					int y = lcdY;
					writeToBuffer(x, y, bgPalette, colorIndex);
				}
			}
		}
	}

	void Ppu::drawSpritesToBuffer()
	{
		if (!gb.ReadFromMemoryMapRegister(HW_LCDC_LCD_CONTROL, LCDC_OBJ_ENABLE))
		{
			return;
		}

		uint8_t lcdY = gb.ReadFromMemoryMap(HW_LY_LCD_Y_COORD);

		RefreshOAMEntries();

		// get oam entries for this scanline
		std::vector<OAM> oamEntriesThisScanLine;
		int oamIndex = 0;

		for (int i = 0; i <= 40; i++)
		{
			if (lcdY >= oamEntries[i].yPos - 16 && lcdY < oamEntries[i].yPos - 8 && oamIndex < 10)
			{
				oamEntriesThisScanLine.push_back(oamEntries[i]);
				oamIndex++;
			}
		}

		uint8_t backgroundTileYOffset = (0 + lcdY) % 8;

		for (int tileX = 0; tileX < 32; tileX++)
		{
			OAM oamOnThisTile;

			for (int i = 0; i < oamEntriesThisScanLine.size(); i++)
			{
				if (oamEntriesThisScanLine[i].xPos == tileX * 8)
				{
					oamOnThisTile = oamEntriesThisScanLine[i];
					break;
				}
			}

			if (oamOnThisTile.address > 0)
			{
				uint16_t tileAddress = 0x8000 + (oamOnThisTile.tileIndex * 16);

				// need to offset the address based on the y position (backgroundTileYOffset) inside the tile
				uint8_t firstByte = gb.ReadFromMemoryMap(tileAddress + (backgroundTileYOffset * 2));
				uint8_t secondByte = gb.ReadFromMemoryMap(tileAddress + (backgroundTileYOffset * 2) + 1);

				// get the obj palette
				uint8_t palette;
				if (oamOnThisTile.paletteOneSelected)
				{
					palette = gb.ReadFromMemoryMap(HW_OBP1_OBJ_PALETTE_1_DATA);
				}
				else
				{
					palette = gb.ReadFromMemoryMap(HW_OBP0_OBJ_PALETTE_0_DATA);
				}

				for (int j = 0; j < 8; j++)
				{
					uint8_t firstBit = (firstByte >> j) & 0x01;
					uint8_t secondBit = (secondByte >> j) & 0x01;
					int colorIndex = (secondBit << 1) | firstBit;

					int x;
					if (oamOnThisTile.xFlip)
					{
						x = (tileX * 8) - j - 1;
					}
					else
					{
						x = (tileX * 8) - j - 1;
					}

					int y = lcdY;
					writeToBuffer(x, y, palette, colorIndex);
				}
			}
		}
	}

	void Ppu::writeToBuffer(int x, int y, uint8_t bgPalette, int colorIndex)
	{
		uint8_t color = bgPalette >> (colorIndex * 2) & 0x03;

		if (x >= 160 || y >= 144)
		{
			return;
			//_ASSERT(x >= 160, "bad ppu!")
		}

		m_backBuffer[y * LCD_WIDTH + x] = color;
	}

	void Ppu::copyBackBufferToLCD()
	{
		for (int y = 0; y < LCD_HEIGHT; y++)
		{
			for (int x = 0; x < LCD_WIDTH; x++)
			{
				m_lcdPixels[y * LCD_WIDTH + x] = m_backBuffer[y * LCD_WIDTH + x];
			}
		}
	}

	void Ppu::clearBackBuffer()
	{
		for (int y = 0; y < LCD_HEIGHT; y++)
		{
			for (int x = 0; x < LCD_WIDTH; x++)
			{
				m_backBuffer[y * LCD_WIDTH + x] = 0;
			}
		}
	}

	// The LCD mode is special because it is contained in two bits within the
	//	STAT register. This seems to be the only place this happens so this 
	//  function exists just to simply the process of retreiving this value
	LCD_Mode Ppu::readLCDMode()
	{
		bool lowBit = gb.ReadFromMemoryMapRegister(HW_STAT_LCD_STATUS, STAT_FLags::STAT_PPU_MODE_LBIT);
		bool highBit = gb.ReadFromMemoryMapRegister(HW_STAT_LCD_STATUS, STAT_FLags::STAT_PPU_MODE_HBIT);

		uint8_t value = 0;
		value |= lowBit ? 0x01 : 0;
		value |= highBit ? 0x02 : 0;

		return static_cast<LCD_Mode>(value);
	}

	void Ppu::writeLCDMode(LCD_Mode mode)
	{
		uint8_t value = static_cast<uint8_t>(mode);
		gb.WriteToMemoryMapRegister(HW_STAT_LCD_STATUS, STAT_FLags::STAT_PPU_MODE_HBIT, (value >> 1) & 0x1);
		gb.WriteToMemoryMapRegister(HW_STAT_LCD_STATUS, STAT_FLags::STAT_PPU_MODE_LBIT, value & 0x1);
	}

	void Ppu::RefreshOAMEntries()
	{
		int i = 0;
		int oamSizeInBytes = 4;
		for (uint16_t addr = 0xFE00; addr <= 0xFE9C; addr += oamSizeInBytes)
		{
			oamEntries[i].address = addr;

			// the first oam byte if the Y Pos
			oamEntries[i].yPos = gb.ReadFromMemoryMap(addr);

			// the second oam byte is the X Pos
			oamEntries[i].xPos = gb.ReadFromMemoryMap(addr + 1);

			// the third oam byte is the tile index
			oamEntries[i].tileIndex = gb.ReadFromMemoryMap(addr + 2);

			// the fourth oam byte is for the attributes
			oamEntries[i].paletteOneSelected = gb.ReadFromMemoryMapRegister(addr + 3, OAM_PALETTE);
			oamEntries[i].xFlip = gb.ReadFromMemoryMapRegister(addr + 3, OAM_FLIP_X);
			oamEntries[i].yFlip = gb.ReadFromMemoryMapRegister(addr + 3, OAM_FLIP_Y);
			oamEntries[i].priority = gb.ReadFromMemoryMapRegister(addr + 3, OAM_PRIORITY);

			i++;
		}
	}
}
