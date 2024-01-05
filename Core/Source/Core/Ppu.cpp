
#include "Ppu.h"
#include "Logger.h"

#include <vector>
#include <algorithm>

namespace Core
{
	Ppu::Ppu(Mmu& mmu) : mmu(mmu) {}
	Ppu::~Ppu() {}

	void Ppu::Clock()
	{
		if (!mmu.ReadRegisterBit(HW_LCDC_LCD_CONTROL, LCDC_Flags::LCDC_LCD_PPU_ENABLE))
		{
			return;
		}

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
			uint8_t lcdy = mmu.Read(HW_LY_LCD_Y_COORD);
			uint8_t lcdyc = mmu.Read(HW_LYC_LY_COMPARE);

			if (lcdy == lcdyc)
			{
				mmu.WriteRegisterBit(HW_STAT_LCD_STATUS, STAT_LYC_EQUAL_LY, true);

				if (mmu.ReadRegisterBit(HW_STAT_LCD_STATUS, STATE_LYC_INT_SELECT))
				{
					mmu.WriteRegisterBit(HW_IF_INTERRUPT_FLAG, IF_LCD, true);
				}
			}
			else
			{
				mmu.WriteRegisterBit(HW_STAT_LCD_STATUS, STAT_LYC_EQUAL_LY, false);
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
			if (mmu.ReadRegisterBit(HW_STAT_LCD_STATUS, STAT_MODE_0_INT_SELECT))
			{
				mmu.WriteRegisterBit(HW_IF_INTERRUPT_FLAG, IF_LCD, true);
			}

			// render graphics
			drawBGToBuffer();
			drawWindowToBuffer();
			drawOAMToBuffer();

			uint8_t ly = mmu.Read(HW_LY_LCD_Y_COORD);
			if (ly == 143)
				writeLCDMode(LCD_Mode::MODE_1_VBLANK);
			else
				writeLCDMode(LCD_Mode::MODE_2_OAMSCAN);

			mmu.Write(HW_LY_LCD_Y_COORD, ly + 1);
			m_CycleCount -= HBLANK_CYCLES;
		}
	}

	void Ppu::processVBlank()
	{
		if (m_CycleCount >= VBLANK_CYCLES)
		{
			uint8_t ly = mmu.Read(HW_LY_LCD_Y_COORD);

			if (ly == 144)
			{
				mmu.WriteRegisterBit(HW_IF_INTERRUPT_FLAG, IF_VBLANK, true);

				copyBackBufferToLCD();
				clearBackBuffer();
			}

			if (ly == 153)
			{
				writeLCDMode(LCD_Mode::MODE_2_OAMSCAN);
				mmu.Write(HW_LY_LCD_Y_COORD, 0);
			}
			else
				mmu.Write(HW_LY_LCD_Y_COORD, ly + 1);

			m_CycleCount -= VBLANK_CYCLES;
		}
	}

	// This will draw one line of the background to the buffer 
	void Ppu::drawBGToBuffer()
	{
		if (!mmu.ReadRegisterBit(HW_LCDC_LCD_CONTROL, LCDC_BG_WINDOW_ENABLE_PRIORITY)) return;

		uint8_t lcdY = mmu.Read(HW_LY_LCD_Y_COORD);
		uint8_t scrollX = mmu.Read(HW_SCX_VIEWPORT_X_POS);
		uint8_t scrollY = mmu.Read(HW_SCY_VIEWPORT_Y_POS);

		// iterate through each pixel
		for (int x = 0; x < 160; x++)
		{
			// get x and y coordinate from within tile map
			uint8_t xCoordInTileMap = (scrollX + x) > TILEMAP_WIDTH_PIXELS ? (scrollX + x) % TILEMAP_WIDTH_PIXELS : (scrollX + x);
			uint8_t yCoordInTileMap = (scrollY + lcdY) > TILEMAP_HEIGHT_PIXELS ? (scrollY + lcdY) % TILEMAP_HEIGHT_PIXELS : (scrollY + lcdY);

			// get tile coordinates
			uint8_t tileXCoord = xCoordInTileMap / TILE_WIDTH;
			uint8_t tileYCoord = yCoordInTileMap / TILE_HEIGHT;

			// get tile x and y offsets
			uint8_t tileXOffset = xCoordInTileMap % TILE_WIDTH;
			uint8_t tileYOffset = yCoordInTileMap % TILE_HEIGHT;

			// get tile at x and y coord
			int tilePosition = tileYCoord * TILEMAP_WIDTH + tileXCoord;

			// get tile id from current tile map
			bool bgTileMapArea = mmu.ReadRegisterBit(HW_LCDC_LCD_CONTROL, LCDC_BG_TILE_MAP);
			uint8_t tileId = mmu.Read(bgTileMapArea ? BG_MAP_1 + tilePosition : BG_MAP_0 + tilePosition);

			// get tile address
			uint16_t tileAddress = GetTileAddressFromTileId(tileId);

			// need to offset the address based on the y position (tileYOffset) inside the tile
			uint8_t firstByte = mmu.Read(tileAddress + (tileYOffset * 2));
			uint8_t secondByte = mmu.Read(tileAddress + (tileYOffset * 2) + 1);

			// get the background palette
			uint8_t bgPalette = mmu.Read(HW_BGP_BG_PALETTE_DATA);

			// because of the bit order and how we are rendering, we need to flip the tileXOffset
			// so we get the right pixel
			int wrappedValue = (7 - tileXOffset) % 8;

			uint8_t firstBit = (firstByte >> wrappedValue) & 0x01;
			uint8_t secondBit = (secondByte >> wrappedValue) & 0x01;
			int colorIndex = (secondBit << 1) | firstBit;

			writeToBuffer(x, lcdY, bgPalette, colorIndex);
		}
	}

	void Ppu::drawWindowToBuffer()
	{
		if (!mmu.ReadRegisterBit(HW_LCDC_LCD_CONTROL, LCDC_WINDOW_ENABLE))
		{
			return;
		}

		uint8_t lcdY = mmu.Read(HW_LY_LCD_Y_COORD);
		int16_t windowX = mmu.Read(HW_WX_WINDOW_X_POS) - 7;
		int16_t windowY = mmu.Read(HW_WY_WINDOW_Y_POS);

		if (lcdY >= windowY)
		{
			// iterate through each pixel
			for (int x = 0; x < 160; x++)
			{
				// get x and y coordinate from within tile map
				uint8_t xCoordInTileMap = x;
				uint8_t yCoordInTileMap = lcdY - windowY;

				// get tile coordinates
				uint8_t tileXCoord = xCoordInTileMap / TILE_WIDTH;
				uint8_t tileYCoord = yCoordInTileMap / TILE_HEIGHT;

				// get tile x and y offsets
				uint8_t tileXOffset = xCoordInTileMap % TILE_WIDTH;
				uint8_t tileYOffset = yCoordInTileMap % TILE_HEIGHT;

				// get tile at x and y coord
				int tilePosition = tileYCoord * TILEMAP_WIDTH + tileXCoord;

				// get tile id from current tile map
				bool bgTileMapArea = mmu.ReadRegisterBit(HW_LCDC_LCD_CONTROL, LCDC_WINDOW_TILE_MAP);
				uint8_t tileId = mmu.Read(bgTileMapArea ? BG_MAP_1 + tilePosition : BG_MAP_0 + tilePosition);

				// get tile address
				uint16_t tileAddress = GetTileAddressFromTileId(tileId);

				// need to offset the address based on the y position (tileYOffset) inside the tile
				uint8_t firstByte = mmu.Read(tileAddress + (tileYOffset * 2));
				uint8_t secondByte = mmu.Read(tileAddress + (tileYOffset * 2) + 1);

				// get the background palette
				uint8_t bgPalette = mmu.Read(HW_BGP_BG_PALETTE_DATA);

				// because of the bit order and how we are rendering, we need to flip the tileXOffset
				// so we get the right pixel
				int wrappedValue = (7 - tileXOffset) % 8;

				uint8_t firstBit = (firstByte >> wrappedValue) & 0x01;
				uint8_t secondBit = (secondByte >> wrappedValue) & 0x01;
				int colorIndex = (secondBit << 1) | firstBit;

				writeToBuffer(x, windowY + (tileYCoord * TILE_HEIGHT) + tileYOffset, bgPalette, colorIndex);
			}
		}
	}

	void Ppu::drawOAMToBuffer()
	{
		if (!mmu.ReadRegisterBit(HW_LCDC_LCD_CONTROL, LCDC_OBJ_ENABLE))
		{
			return;
		}

		uint8_t lcdY = mmu.Read(HW_LY_LCD_Y_COORD);

		RefreshOAMEntries();

		// get oam entries for this scanline
		std::vector<OAM> oamEntriesThisScanLine;
		for (int i = 0; i <= 40; i++)
		{
			if (lcdY >= oamEntries[i].yPos - 16 && 
				((oamEntries[i].isTall && lcdY < oamEntries[i].yPos) || (!oamEntries[i].isTall && lcdY < oamEntries[i].yPos - 8)))
			{
				oamEntriesThisScanLine.push_back(oamEntries[i]);
			}
		}

		// here we sort the tiles based on x position and memory address
		std::sort(oamEntriesThisScanLine.begin(), oamEntriesThisScanLine.end());

		// we want to render the first 10 tiles back to front so that tiles in front will
		//  draw over the tiles in the back.
		for (int i = 10; i > 0; i--)
		{
			// skip if no oam entry exists
			if (i > oamEntriesThisScanLine.size() )
			{
				continue;
			}

			OAM oam = oamEntriesThisScanLine[i - 1];

			uint16_t tileAddress = 0x8000 + (oam.tileIndex * 16);

			// need to offset the address based on the y position (backgroundTileYOffset) inside the tile
			uint8_t backgroundTileYOffset;
			if (oam.yFlip)
			{
				backgroundTileYOffset = (lcdY - (oam.yPos - 16));
				backgroundTileYOffset =  7 - backgroundTileYOffset;
			}
			else
			{
				backgroundTileYOffset = (lcdY - (oam.yPos - 16));
			}

			uint8_t firstByte = mmu.Read(tileAddress + (backgroundTileYOffset * 2));
			uint8_t secondByte = mmu.Read(tileAddress + (backgroundTileYOffset * 2) + 1);

			// get the obj palette
			uint8_t palette;
			if (oam.paletteOneSelected)
			{
				palette = mmu.Read(HW_OBP1_OBJ_PALETTE_1_DATA);
			}
			else
			{
				palette = mmu.Read(HW_OBP0_OBJ_PALETTE_0_DATA);
			}

			for (int p = 0; p < 8; p++)
			{
				uint8_t firstBit = (firstByte >> p) & 0x01;
				uint8_t secondBit = (secondByte >> p) & 0x01;
				int colorIndex = (secondBit << 1) | firstBit;
				
				// color index 0 is considered transparent so ignore it
				if (colorIndex > 0)
				{
					int x;
					if (oam.xFlip)
					{
						x = oam.xPos - 8 + p;
					}
					else
					{
						x = oam.xPos - p - 1;
					}

					int y = lcdY;

					// TODO: It appears we need to look at the color index before the palette is applied
					if ( false /*oam.bgPriority*/)
					{
						int bgColorIndex = ReadFromBuffer(x, y);
						if (bgColorIndex == 0)
						{
							writeToBuffer(x, y, palette, colorIndex);
						}
					}
					else
					{
						writeToBuffer(x, y, palette, colorIndex);
					}
				}
			}
		}
	}

	uint8_t Ppu::ReadFromBuffer(int x, int y)
	{
		if (x >= 160 || y >= 144)
		{
			Logger::Instance().Warning(Domain::PPU, "Attempted bad read from backbuffer");
			return 0;
		}

		return m_backBuffer[y * LCD_WIDTH + x];
	}

	void Ppu::writeToBuffer(int x, int y, uint8_t bgPalette, int colorIndex)
	{
		uint8_t color = bgPalette >> (colorIndex * 2) & 0x03;

		if (x >= 160 || y >= 144)
		{
			Logger::Instance().Warning(Domain::PPU, "Attempted bad write to backbuffer");
			return;
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
		bool lowBit = mmu.ReadRegisterBit(HW_STAT_LCD_STATUS, STAT_FLags::STAT_PPU_MODE_LBIT);
		bool highBit = mmu.ReadRegisterBit(HW_STAT_LCD_STATUS, STAT_FLags::STAT_PPU_MODE_HBIT);

		uint8_t value = 0;
		value |= lowBit ? 0x01 : 0;
		value |= highBit ? 0x02 : 0;

		return static_cast<LCD_Mode>(value);
	}

	void Ppu::writeLCDMode(LCD_Mode mode)
	{
		uint8_t value = static_cast<uint8_t>(mode);
		mmu.WriteRegisterBit(HW_STAT_LCD_STATUS, STAT_FLags::STAT_PPU_MODE_HBIT, (value >> 1) & 0x1);
		mmu.WriteRegisterBit(HW_STAT_LCD_STATUS, STAT_FLags::STAT_PPU_MODE_LBIT, value & 0x1);
	}

	void Ppu::RefreshOAMEntries()
	{
		bool objSize = mmu.ReadRegisterBit(HW_LCDC_LCD_CONTROL, LCDC_OBJ_SIZE);

		int i = 0;
		int oamSizeInBytes = 4;
		for (uint16_t addr = 0xFE00; addr <= 0xFE9C; addr += oamSizeInBytes)
		{
			oamEntries[i].address = addr;

			// the first oam byte if the Y Pos
			oamEntries[i].yPos = mmu.Read(addr);

			// the second oam byte is the X Pos
			oamEntries[i].xPos = mmu.Read(addr + 1);

			// the third oam byte is the tile index
			uint8_t tileIndex = mmu.Read(addr + 2);
			oamEntries[i].tileIndex = objSize ? tileIndex & 0xFE : tileIndex;
			if (oamEntries[i].yFlip && oamEntries[i].isTall)
			{
				oamEntries[i].tileIndex++;
			}

			// the fourth oam byte is for the attributes
			oamEntries[i].paletteOneSelected = mmu.ReadRegisterBit(addr + 3, OAM_PALETTE);
			oamEntries[i].xFlip = mmu.ReadRegisterBit(addr + 3, OAM_FLIP_X);
			oamEntries[i].yFlip = mmu.ReadRegisterBit(addr + 3, OAM_FLIP_Y);
			oamEntries[i].bgPriority = mmu.ReadRegisterBit(addr + 3, OAM_PRIORITY);
			oamEntries[i].isTall = objSize;

			i++;
		}
	}

	Ppu::OAM* Ppu::GetOAMEntries()
	{
		return oamEntries;
	}

	uint16_t Ppu::GetTileAddressFromTileId(uint8_t tileId)
	{
		uint16_t tileAddress;
		bool windowTileDataArea = mmu.ReadRegisterBit(HW_LCDC_LCD_CONTROL, LCDC_BG_AND_WINDOW_TILES);

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

		return tileAddress;
	}
}
