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
		std::vector<uint8_t>* GetRomData() const;

		struct sRomHeader
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
			uint8_t destinationCode;
			uint8_t oldLicenseeCode;
			uint8_t maskRomVersionNumber;
			uint8_t headerChecksum;
			uint8_t globalChecksum[2];
		};

		sRomHeader GetRomInfo() const;
		uint8_t GetRomBank();
		uint8_t GetRamBank();

	private:
		void SaveGameRamToDisk();
		void LoadGameRamFromDisk();
		bool HasBattery();

	private:
		std::string m_RomFileName;
		sRomHeader m_Header;

		bool m_BootRomActive = false;
		bool m_RamEnabled = false;

		uint8_t m_Register_MBC1_Bank1 = 1;
		uint8_t m_Register_MBC1_Bank2 = 0;
		uint8_t m_Register_MBC1_Mode = 0;		
		uint8_t m_BadRamRead = 0xFF;

		std::vector<uint8_t>* m_BootRomData;
		std::vector<uint8_t>* m_RomData;
		std::vector<uint8_t>* m_RamData;
	};
}

