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
		void Write(uint16_t address, uint8_t value);

	public:
		struct sHeader
		{
			uint8_t codeBeginPoint[4];
			uint8_t scrollingNintendoGraphic[48];
			uint8_t title[15];
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

		std::vector<uint8_t>* romData;

	private:
		uint8_t getRomBank();
		uint8_t getRamBank();

	private:
		bool ramEnabled = false;
		uint8_t register_mbc1_bank1 = 0;
		uint8_t register_mbc1_bank2 = 0;
		uint8_t register_mbc1_mode = 0;
		uint8_t ram[32768] = { 0 };
		
		uint8_t badRamRead = 0xFF;
	};
}

