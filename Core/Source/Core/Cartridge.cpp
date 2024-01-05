#include "Cartridge.h"
#include "Defines.h"
#include "Logger.h"
#include <ostream>

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
			int cartSize = (unsigned long)file.tellg();

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
			//// offset address based on current rombank
			//uint8_t romBank = register_mbc1_bank1;// getRomBank();
			//uint16_t romAddress = address + (romBank * ROM_BANK_SIZE);
			//return romData->at(romAddress);

			return romData->at(address + ((getRomBank() - 1) * 0x4000));
		}
		else
		{
			return romData->at(address);
		}
	}

	void Cartridge::Write(uint16_t address, uint8_t value)
	{
		std::ostringstream stream;
		stream << "Cartridge write detected. Address: " << std::hex << address << " Value: " << std::dec << value;
		Logger::Instance().Info(Domain::MMU, stream.str());

		if (address >= MBC1_RAMG_START && address <= MBC1_RAMG_END)
		{
			register_mbc1_ramg = value;
		}
		if (address >= MBC1_BANKREG1_START && address <= MBC1_BANKREG1_END)
		{
			register_mbc1_bank1 = value;
		}
		if (address >= MBC1_BANKREG2_START && address <= MBC1_BANKREG2_END)
		{
			register_mbc1_bank2 = value;
		}
		if (address >= MBC1_MODE_START && address <= MBC1_MODE_END)
		{
			register_mbc1_mode = value;
		}
	}

	uint8_t Cartridge::getRomBank()
	{
		uint8_t bank = register_mbc1_bank1 & 0b11111;
		
		if (bank == 0)
		{
			bank++;
		}

		if (register_mbc1_mode == 0)
		{
			bank |= (register_mbc1_bank2 & 0b11) << 5;
		}

		return bank;
	}
}
