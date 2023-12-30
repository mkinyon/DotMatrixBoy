#include "VRAMViewer.h"
#include "EventManager.h"

#include <algorithm>


namespace App
{
	VRAMViewer::VRAMViewer(Core::GameBoy* gb, SDL_Renderer* renderer) : ImguiWidgetBase("VRAM Viewer"), gameboy(gb)
	{
		EventManager::Instance().Subscribe(Event::VRAM_VIEWER_ENABLE, this);
		EventManager::Instance().Subscribe(Event::VRAM_VIEWER_DISABLE, this);

		tilesTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 192);
		oamTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 8 * 8, 8 * 5);
	}

	VRAMViewer::~VRAMViewer()
	{
		SDL_DestroyTexture(tilesTexture);
		SDL_DestroyTexture(oamTexture);
	}

	void VRAMViewer::RenderContent()
	{
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
		int x = 8;
		int y = 0;

		int count = 0;
		for (uint16_t byte = 0x8000; byte <= 0x97FF; byte += 2)
		{
			uint8_t firstByte = gameboy->ReadFromMemoryMap(byte);
			uint8_t secondByte = gameboy->ReadFromMemoryMap(byte + 1);
			for (int iBit = 0; iBit < 8; iBit++)
			{
				uint8_t firstBit = (firstByte >> iBit) & 0x01;
				uint8_t secondBit = (secondByte >> iBit) & 0x01;
				int colorIndex = (secondBit << 1) | firstBit;
		
				int draw_x = x - iBit; // Adjusted x coordinate for drawing
				int draw_y = y + count; // Adjusted y coordinate for drawing
		
				// get the background palette
				uint8_t bgPalette = gameboy->ReadFromMemoryMap(Core::HW_BGP_BG_PALETTE_DATA);
				uint8_t colorId = bgPalette >> (colorIndex * 2) & 0x03;
		
				int blockIndex = (draw_y * 128 + draw_x) * 4;
				
				//_ASSERT(blockIndex < 0);

				if (colorId == 0)
				{
					tilesTexturePixels[blockIndex] = 255;     // a
					tilesTexturePixels[blockIndex + 1] = 15;  // b
					tilesTexturePixels[blockIndex + 2] = 188; // g
					tilesTexturePixels[blockIndex + 3] = 155; // r
				}
				if (colorId == 1)
				{
					tilesTexturePixels[blockIndex] = 255;
					tilesTexturePixels[blockIndex + 1] = 15;
					tilesTexturePixels[blockIndex + 2] = 172;
					tilesTexturePixels[blockIndex + 3] = 139;
				}
				if (colorId == 2)
				{
					tilesTexturePixels[blockIndex] = 255;
					tilesTexturePixels[blockIndex + 1] = 48;
					tilesTexturePixels[blockIndex + 2] = 98;
					tilesTexturePixels[blockIndex + 3] = 48;
				}
				if (colorId == 3)
				{
					tilesTexturePixels[blockIndex] = 255;
					tilesTexturePixels[blockIndex + 1] = 15;
					tilesTexturePixels[blockIndex + 2] = 56;
					tilesTexturePixels[blockIndex + 3] = 15;
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

		SDL_UpdateTexture(tilesTexture, NULL, tilesTexturePixels, 128 * 4);

		ImVec2 contentSize = ImGui::GetContentRegionAvail();
		float originalWidth = 128.0f; 
		float originalHeight = 192.0f;

		float scaleX = contentSize.x / originalWidth;
		float scaleY = contentSize.y / originalHeight;

		float minScale = std::min(scaleX, scaleY);

		ImVec2 imageSize(originalWidth * minScale, originalHeight * minScale);

		ImGui::Image(reinterpret_cast<ImTextureID>(tilesTexture), imageSize);
	}

	void VRAMViewer::RenderBGMapsView()
	{

	}

	void VRAMViewer::RenderOAMView()
	{
		static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;
		ImVec2 outer_size = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 40);
		if (ImGui::BeginTable("table_scrolly", 10, flags, outer_size))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("OAM Address", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Tile Address", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Tile No", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("X Pos", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Y Pos", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("X Flip", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Y Flip", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Palette", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Priority", ImGuiTableColumnFlags_None);

			ImGui::TableHeadersRow();

			Core::Ppu::OAM* entries = gameboy->ppu.GetOAMEntries();

			for (int row = 0; row < 40; row++)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Image(reinterpret_cast<ImTextureID>(tilesTexture), ImVec2(64,64));
				ImGui::TableNextColumn();
				ImGui::Text("0x%4x", entries[row].address);
				ImGui::TableNextColumn();
				ImGui::Text("0x%4x", entries[row].tileIndex);
				ImGui::TableNextColumn();
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
				ImGui::Text("%d", entries[row].priority);
			}
			ImGui::EndTable();
		}
	}

	void VRAMViewer::OnEvent(Event event)
	{
		if (event == Event::VRAM_VIEWER_ENABLE)
		{
			ShowWindow = true;
		}
		if (event == Event::VRAM_VIEWER_DISABLE)
		{
			ShowWindow = false;
		}
	}
}