#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <cstdint>

namespace Core
{
	class Cartridge
	{
	public:
		Cartridge();
		Cartridge(const std::string& fileName, bool enableBootRom);
		~Cartridge();

	public:
		uint8_t& Read(uint16_t address);

	public:
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
		} Header;

		uint8_t CurrentBankNumber = 1;
		std::vector<uint8_t>* romData;

	private:
		int cartSize = 0;
	};
}

