#include "RomInfo.h"
#include "EventManager.h"
#include "Core/Cartridge.h"

namespace App
{
	RomInfo::RomInfo(Core::GameBoy* gb) : ImguiWidgetBase("Rom Info"), m_GameBoy(gb)
	{
		EventManager::Instance().Subscribe(Event::ROMINFO_ENABLE, this);
		EventManager::Instance().Subscribe(Event::ROMINFO_DISABLE, this);
	}

	RomInfo::~RomInfo() {}

	void RomInfo::RenderContent()
	{
		ImGui::SeparatorText("Rom Header");
		Core::Cartridge* cart = m_GameBoy->GetCart();

		if (cart == nullptr)
		{
			ImGui::Text("Rom Not Loaded");
			return;
		}

		Core::Cartridge::sRomHeader header = cart->GetRomInfo();

		ImGui::Text("Title: %s", header.title); 

		const char* typeName;
		switch (header.cartridgeType)
		{
			case 0x00:	typeName = "ROM ONLY"; break;
			case 0x01:	typeName = "MBC1"; break;
			case 0x02:	typeName = "MBC1 + RAM"; break;
			case 0x03:	typeName = "MBC1 + RAM + BATTERY"; break;
			case 0x05:	typeName = "MBC2"; break;
			case 0x06:	typeName = "MBC2 + BATTERY"; break;
			case 0x08:	typeName = "ROM + RAM 1"; break;
			case 0x09:	typeName = "ROM + RAM + BATTERY 1"; break;
			case 0x0B:	typeName = "MMM01"; break;
			case 0x0C:	typeName = "MMM01 + RAM"; break;
			case 0x0D:	typeName = "MMM01 + RAM + BATTERY"; break;
			case 0x0F:	typeName = "MBC3 + TIMER + BATTERY"; break;
			case 0x10:	typeName = "MBC3 + TIMER + RAM + BATTERY 2"; break;
			case 0x11:	typeName = "MBC3"; break;
			case 0x12:	typeName = "MBC3 + RAM 2"; break;
			case 0x13:	typeName = "MBC3 + RAM + BATTERY 2"; break;
			case 0x19:	typeName = "MBC5"; break;
			case 0x1A:	typeName = "MBC5 + RAM"; break;
			case 0x1B:	typeName = "MBC5 + RAM + BATTERY"; break;
			case 0x1C:	typeName = "MBC5 + RUMBLE"; break;
			case 0x1D:	typeName = "MBC5 + RUMBLE + RAM"; break;
			case 0x1E:	typeName = "MBC5 + RUMBLE + RAM + BATTERY"; break;
			case 0x20:	typeName = "MBC6"; break;
			case 0x22:	typeName = "MBC7 + SENSOR + RUMBLE + RAM + BATTERY"; break;
			case 0xFC:	typeName = "POCKET CAMERA"; break;
			case 0xFD:	typeName = "BANDAI TAMA5"; break;
			case 0xFE:	typeName = "HuC3"; break;
			case 0xFF:	typeName = "HuC1 + RAM + BATTERY"; break;
		};
		
		ImGui::Text("Cart Type: %s", typeName);

		const char* sizeDescription;
		int numRomBanks;
		switch (header.romSize)
		{
			case 0x00: sizeDescription = "32 KiB (No Banking)";	numRomBanks = 2;   break;
			case 0x01: sizeDescription = "64 KiB";				numRomBanks = 4;   break;
			case 0x02: sizeDescription = "128 KiB";				numRomBanks = 8;   break;
			case 0x03: sizeDescription = "256 KiB";				numRomBanks = 16;  break;
			case 0x04: sizeDescription = "512 KiB";				numRomBanks = 32;  break;
			case 0x05: sizeDescription = "1 MiB";				numRomBanks = 64;  break;
			case 0x06: sizeDescription = "2 MiB";				numRomBanks = 128; break;
			case 0x07: sizeDescription = "4 MiB";				numRomBanks = 256; break;
			case 0x08: sizeDescription = "8 MiB";				numRomBanks = 512; break;
			case 0x52: sizeDescription = "1.1 MiB";				numRomBanks = 72;  break;
			case 0x53: sizeDescription = "1.2 MiB";				numRomBanks = 80;  break;
			case 0x54: sizeDescription = "1.5 MiB";				numRomBanks = 96;  break;
		};

		ImGui::Text("Rom Size: %s", sizeDescription);
		ImGui::Text("Number of Rom Banks: %d", numRomBanks);

		const char* sramSize;
		const char* ramComment;
		switch (header.ramSize)
		{
			case 0x00: sramSize = "0";			ramComment = "No RAM";					break;
			case 0x01: sramSize = "–";			ramComment = "Unused";					break;
			case 0x02: sramSize = "8 KiB";		ramComment = "1 bank";					break;
			case 0x03: sramSize = "32 KiB";		ramComment = "4 banks of 8 KiB each";   break;
			case 0x04: sramSize = "128 KiB";	ramComment = "16 banks of 8 KiB each";  break;
			case 0x05: sramSize = "64 KiB";		ramComment = "8 banks of 8 KiB each";   break;
		}

		ImGui::Text("Ram Size: %s (%s)", sramSize, ramComment);

		ImGui::SeparatorText("Cartridge State");
		
		ImGui::Text("Current Rom Bank: %d", cart->GetRomBank());
		ImGui::Text("Current Ram Bank: %d", cart->GetRamBank());
	}

	void RomInfo::OnEvent(Event event)
	{
		if (event == Event::DEBUGGER_ENABLE)
		{
			m_ShowWindow = true;
		}
		if (event == Event::DEBUGGER_DISABLE)
		{
			m_ShowWindow = false;
		}
	}
}