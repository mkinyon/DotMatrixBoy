#include "Cartridge.h"

namespace Core
{
	Cartridge::Cartridge() {}

	Cartridge::Cartridge(const std::string& fileName, bool enableBootRom)
	{
		std::ifstream file;
		file.open(fileName, std::ifstream::binary);
		if (file.is_open())
		{
			// read header
			file.seekg(0x100); // the header starts at location $100
			file.read((char*)&Header, sizeof(sHeader));

			file.seekg(0, file.end);
			cartSize = (unsigned long)file.tellg();

			file.seekg(0); // reset to beginning
			romData = new std::vector<uint8_t>(cartSize);
			file.read((char*)romData->data(), cartSize);

		}

		// load bootrom
		// TODO: I believe the bootrom is only enabled during the boot process.
		//	Afterwards, the bootrom is unmapped. idk.
		if (enableBootRom)
		{
			std::ifstream bootrom;
			bootrom.open("../Roms/dmg_boot.bin", std::ifstream::binary);
			if (bootrom.is_open())
			{
				bootrom.seekg(0);
				bootrom.read((char*)romData->data(), 256);
			}
		}
	}

	Cartridge::~Cartridge() {}

	uint8_t& Cartridge::Read(uint16_t address)
	{
		if (address >= 0x4000 && address <= 0x7FFF)
		{
			// offset address based on current rombank
			return romData->at(address + ((CurrentBankNumber - 1) * 0x4000));
		}
		else
		{
			return romData->at(address);
		}
	}
}
