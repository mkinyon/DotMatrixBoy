
#include "Ppu.h"
#include "Logger.h"

#include <vector>
#include <algorithm>

namespace Core
{
	Ppu::Ppu(Mmu& mmu) : m_MMU(mmu) {}
	Ppu::~Ppu() {}

	void Ppu::Clock()
	{
		if (!m_MMU.ReadRegisterBit(HW_FF40_LCDC_LCD_CONTROL, LCDC_Flags::LCDC_LCD_PPU_ENABLE))
		{
			return;
		}

		LCD_Mode currentMode = ReadLCDMode();
		m_CycleCount++;

		switch (currentMode)
		{
			case LCD_Mode::MODE_2_OAMSCAN:
				ProcessOAM();
				break;
			case LCD_Mode::MODE_3_DRAWING:
				ProcessDrawing();
				break;
			case LCD_Mode::MODE_0_HBLANK:
				ProcessHBlank();
				break;
			case LCD_Mode::MODE_1_VBLANK:
				ProcessVBlank();
				break;
		}
	}

	void Ppu::Reset()
	{
		m_LCDMode = LCD_Mode::MODE_0_HBLANK;
		m_CycleCount = 0;
		m_TotalFrames = 0;
		m_CurrentScanLine = 0;
		m_TotalDotsThisFrame = 0;
	}

	LCD_Mode Ppu::GetMode()
	{
		return ReadLCDMode();
	}

	void Ppu::ProcessOAM()
	{
		if (m_CycleCount >= OAM_CYCLES)
		{
			// Check for OAM interrupt
			if (m_MMU.ReadRegisterBit(HW_FF41_STAT_LCD_STATUS, STAT_MODE_2_INT_SELECT))
			{
				m_MMU.WriteRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, IF_LCD, true);
			}

			ProcessLYC();

			WriteLCDMode(LCD_Mode::MODE_3_DRAWING);
			m_CycleCount -= OAM_CYCLES;
		}
	}

	void Ppu::ProcessDrawing()
	{
		if (m_CycleCount >= DRAWING_CYCLES)
		{
			WriteLCDMode(LCD_Mode::MODE_0_HBLANK);
			m_CycleCount -= DRAWING_CYCLES;
		}
	}

	void Ppu::ProcessHBlank()
	{
		if (m_CycleCount >= HBLANK_CYCLES)
		{
			ProcessLYC();

			// Check for H-BLANK interrupt
			if (m_MMU.ReadRegisterBit(HW_FF41_STAT_LCD_STATUS, STAT_MODE_0_INT_SELECT))
			{
				m_MMU.WriteRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, IF_LCD, true);
			}

			// render graphics
			DrawBGToBuffer();
			DrawWindowToBuffer();
			DrawOAMToBuffer();

			uint8_t ly = m_MMU.Read(HW_FF44_LY_LCD_Y_COORD, true);
			if (ly == 143)
				WriteLCDMode(LCD_Mode::MODE_1_VBLANK);
			else
				WriteLCDMode(LCD_Mode::MODE_2_OAMSCAN);

			m_MMU.Write(HW_FF44_LY_LCD_Y_COORD, m_MMU.Read(HW_FF44_LY_LCD_Y_COORD, true) + 1, true);
			m_CurrentScanLine = ly + 1;
			m_CycleCount -= HBLANK_CYCLES;
		}
	}

	void Ppu::ProcessVBlank()
	{
		if (m_CycleCount >= VBLANK_CYCLES)
		{
			ProcessLYC();

			uint8_t ly = m_MMU.Read(HW_FF44_LY_LCD_Y_COORD, true);

			if (ly == 144)
			{
				m_MMU.WriteRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, IF_VBLANK, true);

				CopyBackBufferToLCD();
				ClearBackBuffer();
			}

			if (ly == 153)
			{
				WriteLCDMode(LCD_Mode::MODE_2_OAMSCAN);
				m_MMU.Write(HW_FF44_LY_LCD_Y_COORD, 0, true);
				m_CurrentScanLine = 0;
				m_TotalFrames++;
				ProcessLYC();
			}
			else
			{
				m_MMU.Write(HW_FF44_LY_LCD_Y_COORD, ly + 1, true);
				m_CurrentScanLine = ly + 1;
			}
				

			m_CycleCount -= VBLANK_CYCLES;
		}
	}

	// This will draw one line of the background to the buffer 
	void Ppu::DrawBGToBuffer()
	{
		if (!m_MMU.ReadRegisterBit(HW_FF40_LCDC_LCD_CONTROL, LCDC_BG_WINDOW_ENABLE_PRIORITY)) return;

		uint8_t lcdY = m_MMU.Read(HW_FF44_LY_LCD_Y_COORD, true);
		uint8_t scrollX = m_MMU.Read(HW_FF43_SCX_VIEWPORT_X_POS, true);
		uint8_t scrollY = m_MMU.Read(HW_FF42_SCY_VIEWPORT_Y_POS, true);

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
			bool bgTileMapArea = m_MMU.ReadRegisterBit(HW_FF40_LCDC_LCD_CONTROL, LCDC_BG_TILE_MAP);
			uint8_t tileId = m_MMU.Read(bgTileMapArea ? BG_MAP_1 + tilePosition : BG_MAP_0 + tilePosition);

			// get tile address
			uint16_t tileAddress = GetTileAddressFromTileId(tileId);

			// need to offset the address based on the y position (tileYOffset) inside the tile
			uint8_t firstByte = m_MMU.Read(tileAddress + (tileYOffset * 2));
			uint8_t secondByte = m_MMU.Read(tileAddress + (tileYOffset * 2) + 1);

			// get the background palette
			uint8_t bgPalette = m_MMU.Read(HW_FF47_BGP_BG_PALETTE_DATA);

			// because of the bit order and how we are rendering, we need to flip the tileXOffset
			// so we get the right pixel
			int wrappedValue = (7 - tileXOffset) % 8;

			uint8_t firstBit = (firstByte >> wrappedValue) & 0x01;
			uint8_t secondBit = (secondByte >> wrappedValue) & 0x01;
			int colorIndex = (secondBit << 1) | firstBit;

			WriteToBuffer(x, lcdY, bgPalette, colorIndex);
		}
	}

	void Ppu::DrawWindowToBuffer()
	{
		if (!m_MMU.ReadRegisterBit(HW_FF40_LCDC_LCD_CONTROL, LCDC_WINDOW_ENABLE))
		{
			return;
		}

		uint8_t lcdY = m_MMU.Read(HW_FF44_LY_LCD_Y_COORD, true);
		int16_t windowX = m_MMU.Read(HW_FF4B_WX_WINDOW_X_POS, true) - 7;
		int16_t windowY = m_MMU.Read(HW_FF4A_WY_WINDOW_Y_POS, true);

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
				bool bgTileMapArea = m_MMU.ReadRegisterBit(HW_FF40_LCDC_LCD_CONTROL, LCDC_WINDOW_TILE_MAP);
				uint8_t tileId = m_MMU.Read(bgTileMapArea ? BG_MAP_1 + tilePosition : BG_MAP_0 + tilePosition);

				// get tile address
				uint16_t tileAddress = GetTileAddressFromTileId(tileId);

				// need to offset the address based on the y position (tileYOffset) inside the tile
				uint8_t firstByte = m_MMU.Read(tileAddress + (tileYOffset * 2));
				uint8_t secondByte = m_MMU.Read(tileAddress + (tileYOffset * 2) + 1);

				// get the background palette
				uint8_t bgPalette = m_MMU.Read(HW_FF47_BGP_BG_PALETTE_DATA);

				// because of the bit order and how we are rendering, we need to flip the tileXOffset
				// so we get the right pixel
				int wrappedValue = (7 - tileXOffset) % 8;

				uint8_t firstBit = (firstByte >> wrappedValue) & 0x01;
				uint8_t secondBit = (secondByte >> wrappedValue) & 0x01;
				int colorIndex = (secondBit << 1) | firstBit;

				WriteToBuffer(x, windowY + (tileYCoord * TILE_HEIGHT) + tileYOffset, bgPalette, colorIndex);
			}
		}
	}

	void Ppu::DrawOAMToBuffer()
	{
		if (!m_MMU.ReadRegisterBit(HW_FF40_LCDC_LCD_CONTROL, LCDC_OBJ_ENABLE))
		{
			return;
		}

		uint8_t lcdY = m_MMU.Read(HW_FF44_LY_LCD_Y_COORD, true);

		RefreshOAMEntries();

		// get oam entries for this scanline
		std::vector<OAM> oamEntriesThisScanLine;
		for (int i = 0; i < 40; i++)
		{
			if (lcdY >= m_OAMEntries[i].yPos - 16 && 
				((m_OAMEntries[i].isTall && lcdY < m_OAMEntries[i].yPos) || (!m_OAMEntries[i].isTall && lcdY < m_OAMEntries[i].yPos - 8)))
			{
				oamEntriesThisScanLine.push_back(m_OAMEntries[i]);
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

			uint8_t firstByte = m_MMU.Read(tileAddress + (backgroundTileYOffset * 2));
			uint8_t secondByte = m_MMU.Read(tileAddress + (backgroundTileYOffset * 2) + 1);

			// get the obj palette
			uint8_t palette;
			if (oam.paletteOneSelected)
			{
				palette = m_MMU.Read(HW_FF49_OBP1_OBJ_PALETTE_1_DATA);
			}
			else
			{
				palette = m_MMU.Read(HW_FF48_OBP0_OBJ_PALETTE_0_DATA);
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
					// BG and Window colors 1�3 are drawn over this OBJ
					if ( false /* oam.bgPriority */)
					{
						int bgColorIndex = ReadFromBuffer(x, y);
						if (bgColorIndex == 0)
						{
							WriteToBuffer(x, y, palette, colorIndex);
						}
					}
					else
					{
						WriteToBuffer(x, y, palette, colorIndex);
					}
				}
			}
		}
	}

	uint8_t Ppu::ReadFromBuffer(int x, int y)
	{
		if (x < 0 || x >= 160 || y < 0 || y >= 144)
		{
			Logger::Instance().Warning(Domain::PPU, "Attempted bad read from backbuffer");
			return 0;
		}

		return m_BackBuffer[y * LCD_WIDTH + x];
	}

	void Ppu::WriteToBuffer(int x, int y, uint8_t bgPalette, int colorIndex)
	{
		uint8_t color = bgPalette >> (colorIndex * 2) & 0x03;

		if (x < 0 || x >= 160 || y < 0 || y >= 144)
		{
			Logger::Instance().Warning(Domain::PPU, "Attempted bad write to backbuffer");
			return;
		}

		m_BackBuffer[y * LCD_WIDTH + x] = color;
	}

	void Ppu::CopyBackBufferToLCD()
	{
		for (int y = 0; y < LCD_HEIGHT; y++)
		{
			for (int x = 0; x < LCD_WIDTH; x++)
			{
				m_LCDPixels[y * LCD_WIDTH + x] = m_BackBuffer[y * LCD_WIDTH + x];
			}
		}
	}

	void Ppu::ClearBackBuffer()
	{
		for (int y = 0; y < LCD_HEIGHT; y++)
		{
			for (int x = 0; x < LCD_WIDTH; x++)
			{
				m_BackBuffer[y * LCD_WIDTH + x] = 0;
			}
		}
	}

	// The LCD mode is special because it is contained in two bits within the
	//	STAT register. This seems to be the only place this happens so this 
	//  function exists just to simply the process of retreiving this value
	LCD_Mode Ppu::ReadLCDMode()
	{
		return m_LCDMode;
	}

	void Ppu::WriteLCDMode(LCD_Mode mode)
	{
		uint8_t value = static_cast<uint8_t>(mode);
		m_MMU.WriteRegisterBit(HW_FF41_STAT_LCD_STATUS, STAT_FLags::STAT_PPU_MODE_HBIT, (value >> 1) & 0x1);
		m_MMU.WriteRegisterBit(HW_FF41_STAT_LCD_STATUS, STAT_FLags::STAT_PPU_MODE_LBIT, value & 0x1);

		m_LCDMode = mode;
	}

	void Ppu::RefreshOAMEntries()
	{
		bool objSize = m_MMU.ReadRegisterBit(HW_FF40_LCDC_LCD_CONTROL, LCDC_OBJ_SIZE);

		int i = 0;
		int oamSizeInBytes = 4;
		for (uint16_t addr = 0xFE00; addr <= 0xFE9C; addr += oamSizeInBytes)
		{
			m_OAMEntries[i].address = addr;

			// the first oam byte if the Y Pos
			m_OAMEntries[i].yPos = m_MMU.Read(addr);

			// the second oam byte is the X Pos
			m_OAMEntries[i].xPos = m_MMU.Read(addr + 1);

			// the third oam byte is the tile index
			uint8_t tileIndex = m_MMU.Read(addr + 2);
			m_OAMEntries[i].tileIndex = objSize ? tileIndex & 0xFE : tileIndex;
			if (m_OAMEntries[i].yFlip && m_OAMEntries[i].isTall)
			{
				m_OAMEntries[i].tileIndex++;
			}

			// the fourth oam byte is for the attributes
			m_OAMEntries[i].paletteOneSelected = m_MMU.ReadRegisterBit(addr + 3, OAM_PALETTE);
			m_OAMEntries[i].xFlip = m_MMU.ReadRegisterBit(addr + 3, OAM_FLIP_X);
			m_OAMEntries[i].yFlip = m_MMU.ReadRegisterBit(addr + 3, OAM_FLIP_Y);
			m_OAMEntries[i].bgPriority = m_MMU.ReadRegisterBit(addr + 3, OAM_PRIORITY);
			m_OAMEntries[i].isTall = objSize;

			i++;
		}
	}

	Ppu::OAM* Ppu::GetOAMEntries()
	{
		return m_OAMEntries;
	}

	uint16_t Ppu::GetTileAddressFromTileId(uint8_t tileId)
	{
		uint16_t tileAddress;
		bool windowTileDataArea = m_MMU.ReadRegisterBit(HW_FF40_LCDC_LCD_CONTROL, LCDC_BG_AND_WINDOW_TILES);

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

	void Ppu::ProcessLYC()
	{
		uint8_t lcdy = m_MMU.Read(HW_FF44_LY_LCD_Y_COORD, true);
		uint8_t lcdyc = m_MMU.Read(HW_FF45_LYC_LY_COMPARE, true);

		if (lcdy == lcdyc)
		{
			m_MMU.WriteRegisterBit(HW_FF41_STAT_LCD_STATUS, STAT_LYC_EQUAL_LY, true);
		}
		else
		{
			m_MMU.WriteRegisterBit(HW_FF41_STAT_LCD_STATUS, STAT_LYC_EQUAL_LY, false);
		}

		if (m_MMU.ReadRegisterBit(HW_FF41_STAT_LCD_STATUS, STATE_LYC_INT_SELECT) &&
			m_MMU.ReadRegisterBit(HW_FF41_STAT_LCD_STATUS, STAT_LYC_EQUAL_LY))
		{
			m_MMU.WriteRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, IF_LCD, true);
		}
	}

	int Ppu::GetTotalFrames()
	{
		return m_TotalFrames;
	}

	int Ppu::GetCurrentScanLine()
	{
		return m_CurrentScanLine;
	}

	int Ppu::GetTotalDotsThisFrame()
	{
		return m_TotalDotsThisFrame;
	}

	uint8_t* Ppu::GetLCDPixels()
	{
		return m_LCDPixels;
	}
}
