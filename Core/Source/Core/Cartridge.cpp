#include "Cartridge.h"

namespace Core
{
	Cartridge::Cartridge() {}

	Cartridge::Cartridge(const std::string& fileName, bool enableBootRom)
	{
		// GB header
		struct sHeader
		{
			uint8_t codeBeginPoint[4];
			uint8_t scrollingNintendoGraphic[48];
			uint8_t title[10];
			uint8_t cGBFlag;
			uint8_t newLicenseeCode[2];
			uint8_t sGBFlag;
			uint8_t cartridgeType;
			uint8_t romSize;
			uint8_t ramSize;
			uint8_t desitinationCode;
			uint8_t oldLicenseeCode;
			uint8_t maskRomVersionNumber;
			uint8_t headerChecksum;
			uint8_t globalChecksum[2];
		} header;

		std::ifstream file;
		file.open(fileName, std::ifstream::binary);
		if (file.is_open())
		{
			// read header
			file.seekg(0x100); // the header starts at location $100
			file.read((char*)&header, sizeof(sHeader));

			// ROM ONLY cartridge type
			if (header.cartridgeType == 0x00)
			{
				file.seekg(0); // reset to beginning
				romData = new std::vector<uint8_t>(32768);
				file.read((char*)romData->data(), romData->size());
			}
		}

		// load bootrom
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
		return romData->at(address);
	}
}