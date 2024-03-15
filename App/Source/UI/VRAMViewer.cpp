#include "VRAMViewer.h"
#include "EventManager.h"
#include "Core\Defines.h"
#include <algorithm>


namespace App
{
	VRAMViewer::VRAMViewer(Core::GameBoy* gb, SDL_Renderer* renderer) : ImguiWidgetBase("VRAM Viewer"), m_GameBoy(gb)
	{
		EventManager::Instance().Subscribe(Event::VRAM_VIEWER_ENABLE, this);
		EventManager::Instance().Subscribe(Event::VRAM_VIEWER_DISABLE, this);

		m_TilesTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 192);
	}

	VRAMViewer::~VRAMViewer()
	{
		SDL_DestroyTexture(m_TilesTexture);
	}

	void VRAMViewer::RenderContent()
	{
		updateTexture();
		static WindowState activeWindowState = WindowState::Tiles;

		if (ImGui::Button("Tiles", ImVec2(ImGui::GetContentRegionMax().x * 0.33f - 6, 20)))
		{
			activeWindowState = WindowState::Tiles;
		}
		
		ImGui::SameLine();

		if (ImGui::Button("BG Maps", ImVec2(ImGui::GetContentRegionMax().x * 0.33f - 6, 20)))
		{
			activeWindowState = WindowState::BGMaps;
		}
		
		ImGui::SameLine();

		if (ImGui::Button("OAM", ImVec2(ImGui::GetContentRegionMax().x * 0.33f - 6, 20)))
		{
			activeWindowState = WindowState::OAM;
		}
	

		ImGui::Separator();

		switch (activeWindowState)
		{
			case WindowState::Tiles:
				RenderTilesView();
				break;
			case WindowState::BGMaps:
				RenderBGMapsView();
				break;
			case WindowState::OAM:
				RenderOAMView();
				break;
		}
		
	}

	void VRAMViewer::RenderTilesView()
	{
		ImVec2 contentSize = ImGui::GetContentRegionAvail();
		float originalWidth = 128.0f; 
		float originalHeight = 192.0f;

		float scaleX = contentSize.x / originalWidth;
		float scaleY = contentSize.y / originalHeight;

		float minScale = std::min(scaleX, scaleY);

		ImVec2 imageSize(originalWidth * minScale, originalHeight * minScale);

		ImGui::Image(reinterpret_cast<ImTextureID>(m_TilesTexture), imageSize);
	}

	void VRAMViewer::RenderBGMapsView()
	{
		bool bgTileMapArea = m_GameBoy->m_MMU.ReadRegisterBit(Core::HW_LCDC_LCD_CONTROL, Core::LCDC_BG_TILE_MAP);
		bool windowTileDataArea = m_GameBoy->m_MMU.ReadRegisterBit(Core::HW_LCDC_LCD_CONTROL, Core::LCDC_BG_AND_WINDOW_TILES);
		ImGui::Text("Current BG Map 1?: %2d", bgTileMapArea);

		// set custom item spacing for this section
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec2 originalSpacing = style.ItemSpacing;
		style.ItemSpacing = ImVec2(1.0f, 1.0f);

		for (int y = 0; y < 32; y++)
		{
			for (int x = 0; x < 32; x++)
			{
				// get tile at x and y coord
				int tilePosition = y * Core::TILEMAP_WIDTH + x;

				// get tile id from current tile map
				uint8_t tileId = m_GameBoy->m_MMU.Read(bgTileMapArea ? Core::BG_MAP_1 + tilePosition : Core::BG_MAP_0 + tilePosition);

				int yOffset = 0;
				if (!windowTileDataArea)
				{
					if (tileId >= 128)
					{
						yOffset = 0;

					}
					else
					{
						yOffset = 16;
					}
				}

				int texCoordX = (tileId % 16) * 8;
				int texCoordY = ((tileId / 16) + yOffset) * 8;

				ImVec2 uvStart = ImVec2(texCoordX / 128.0f, texCoordY / 192.0f);
				ImVec2 uvEnd = ImVec2((texCoordX + 8) / 128.0f, (texCoordY + 8) / 192.0f);

				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::Text("X: %2d Y: %2d", x, y);
					ImGui::EndTooltip();
				}
				ImGui::Image(reinterpret_cast<ImTextureID>(m_TilesTexture), ImVec2(16, 16), uvStart, uvEnd);
				if (x < 31) 
				{
					ImGui::SameLine();
				}
			}
		}

		// restore the original spacing
		style.ItemSpacing = originalSpacing;
	}

	void VRAMViewer::RenderOAMView()
	{
		static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;
		ImVec2 outer_size = ImVec2(0.0f, ImGui::GetContentRegionAvail().y);
		if (ImGui::BeginTable("table_scrolly", 10, flags, outer_size))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("OAM Address", ImGuiTableColumnFlags_None);
			//ImGui::TableSetupColumn("Tile Address", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Tile No", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("X Pos", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Y Pos", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("X Flip", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Y Flip", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Palette", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Priority", ImGuiTableColumnFlags_None);

			ImGui::TableHeadersRow();

			Core::Ppu::OAM* entries = m_GameBoy->m_PPU.GetOAMEntries();

			for (int row = 0; row < 40; row++)
			{
				float startX = static_cast<float>((entries[row].tileIndex * 8) % 128);
				float startY = static_cast<float>(((entries[row].tileIndex * 8) / 128) * 8);

				ImVec2 uvStart = ImVec2(startX / 128.0f, startY / 192.0f);
				ImVec2 uvEnd = ImVec2((startX + 8) / 128.0f, (startY + 8) / 192.0f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				if (entries[row].yPos > 0 && entries[row].xPos > 0)
				{
					ImGui::Image(reinterpret_cast<ImTextureID>(m_TilesTexture), ImVec2(32, 32), uvStart, uvEnd);
				}
				ImGui::TableNextColumn();
				ImGui::Text("0x%4x", entries[row].address);
				ImGui::TableNextColumn();
				//ImGui::Text("0x%4x", entries[row].tileIndex);
				//ImGui::TableNextColumn();
				ImGui::Text("%2x", entries[row].tileIndex);
				ImGui::TableNextColumn();
				ImGui::Text("%d", entries[row].xPos);
				ImGui::TableNextColumn();
				ImGui::Text("%d", entries[row].yPos);
				ImGui::TableNextColumn();
				ImGui::Text("%d", entries[row].xFlip);
				ImGui::TableNextColumn();
				ImGui::Text("%d", entries[row].yFlip);
				ImGui::TableNextColumn();
				ImGui::Text("%d", entries[row].paletteOneSelected);
				ImGui::TableNextColumn();
				ImGui::Text("%d", entries[row].bgPriority);
			}
			ImGui::EndTable();
		}
	}

	void VRAMViewer::updateTexture()
	{
		uint8_t bgPalette = m_GameBoy->m_MMU.Read(Core::HW_BGP_BG_PALETTE_DATA);

		int x = 8;
		int y = 0;

		int count = 0;
		for (uint16_t byte = 0x8000; byte <= 0x97FF; byte += 2)
		{
			uint8_t firstByte = m_GameBoy->m_MMU.Read(byte);
			uint8_t secondByte = m_GameBoy->m_MMU.Read(byte + 1);
			for (int iBit = 0; iBit < 8; iBit++)
			{
				uint8_t firstBit = (firstByte >> iBit) & 0x01;
				uint8_t secondBit = (secondByte >> iBit) & 0x01;
				int colorIndex = (secondBit << 1) | firstBit;

				int draw_x = x - iBit; // Adjusted x coordinate for drawing
				int draw_y = y + count; // Adjusted y coordinate for drawing
				
				uint8_t colorId = bgPalette >> (colorIndex * 2) & 0x03;
				int blockIndex = (draw_y * 128 + draw_x - 1) * 4;

				if (colorId == 0)
				{
					m_TilesTexturePixels[blockIndex] = 255;     // a
					m_TilesTexturePixels[blockIndex + 1] = 15;  // b
					m_TilesTexturePixels[blockIndex + 2] = 188; // g
					m_TilesTexturePixels[blockIndex + 3] = 155; // r
				}
				if (colorId == 1)
				{
					m_TilesTexturePixels[blockIndex] = 255;
					m_TilesTexturePixels[blockIndex + 1] = 15;
					m_TilesTexturePixels[blockIndex + 2] = 172;
					m_TilesTexturePixels[blockIndex + 3] = 139;
				}
				if (colorId == 2)
				{
					m_TilesTexturePixels[blockIndex] = 255;
					m_TilesTexturePixels[blockIndex + 1] = 48;
					m_TilesTexturePixels[blockIndex + 2] = 98;
					m_TilesTexturePixels[blockIndex + 3] = 48;
				}
				if (colorId == 3)
				{
					m_TilesTexturePixels[blockIndex] = 255;
					m_TilesTexturePixels[blockIndex + 1] = 15;
					m_TilesTexturePixels[blockIndex + 2] = 56;
					m_TilesTexturePixels[blockIndex + 3] = 15;
				}
			}
			count++;

			if ((count % 128) == 0)
			{
				x -= 120;
			}
			else if ((count % 8) == 0)
			{
				x += 8; // Move x coordinate right by 8
				y -= 8;
			}
		}

		SDL_UpdateTexture(m_TilesTexture, NULL, m_TilesTexturePixels, 128 * 4);
	}

	void VRAMViewer::OnEvent(Event event)
	{
		if (event == Event::VRAM_VIEWER_ENABLE)
		{
			m_ShowWindow = true;
		}
		if (event == Event::VRAM_VIEWER_DISABLE)
		{
			m_ShowWindow = false;
		}
	}
}