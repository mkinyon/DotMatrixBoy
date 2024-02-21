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
			m_RomData = new std::vector<uint8_t>(cartSize);
			file.read((char*)m_RomData->data(), cartSize);

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
				bootrom.read((char*)m_RomData->data(), 256);
			}
		}
	}

	Cartridge::~Cartridge() {}

	uint8_t& Cartridge::Read(uint16_t address)
	{
		if (address >= CARTBANK_ADDR_RANGE_START && address <= CARTBANK_ADDR_RANGE_END)
		{
			// offset address based on current rombank
			uint8_t romBank = GetRomBank();
			uint16_t romAddress = address + (romBank * ROM_BANK_SIZE);
			//return romData->at(romAddress);

			return m_RomData->at(address + ((GetRomBank() - 1) * 0x4000));
		}
		else if (address >= CARTRAM_ADDR_RANGE_START && address <= CARTRAM_ADDR_RANGE_END)
		{
			if (m_RamEnabled)
			{
				uint8_t ramBank = GetRamBank();
				uint16_t ramAddress = address + (ramBank * RAM_BANK_SIZE);
				uint16_t addrOffset = ramAddress - 0xA000;
				return m_RAM[addrOffset];
			}
			else
			{
				return m_BadRamRead;
			}
		}
		else
		{
			return m_RomData->at(address);
		}
	}

	void Cartridge::Write(uint16_t address, uint8_t value)
	{
		std::ostringstream stream;
		stream << "Cartridge write detected. Address: " << std::hex << address << " Value: " << std::setw(2) << std::setfill('0') << static_cast<int>(value);
		Logger::Instance().Info(Domain::MMU, stream.str());

		if (address >= MBC1_RAMG_START && address <= MBC1_RAMG_END)
		{
			// writing 0xA (0b1010) to this address range will enable ram
			if (value == 0xA)
			{
				m_RamEnabled = true;
			}
			// all other writes will disable it
			else
			{
				m_RamEnabled = false;
			}
		}
		if (address >= MBC1_BANKREG1_START && address <= MBC1_BANKREG1_END)
		{
			m_Register_MBC1_Bank1 = value;
		}
		if (address >= MBC1_BANKREG2_START && address <= MBC1_BANKREG2_END)
		{
			m_Register_MBC1_Bank2 = value;
		}
		if (address >= MBC1_MODE_START && address <= MBC1_MODE_END)
		{
			m_Register_MBC1_Mode = value & 0x1;
		}
		if (address >= CARTRAM_ADDR_RANGE_START && address <= CARTRAM_ADDR_RANGE_END)
		{
			if (/*register_mbc1_mode == 1 &&*/ m_RamEnabled)
			{
				uint8_t ramBank = GetRamBank();
				uint16_t ramAddress = address + (ramBank * RAM_BANK_SIZE);
				uint16_t addrOffset = ramAddress - 0xA000;
				m_RAM[addrOffset] = value;
			}
		}
	}

	uint8_t Cartridge::GetRomBank()
	{
		// only the lower 5 bits count
		uint8_t bank = m_Register_MBC1_Bank1 & 0b11111;

		// the lower five bits can't be all zero, it has to be at least 1
		if (bank == 0)
		{
			bank++;
		}

		// 32KiB roms do not use rom banking so hardcode it to one
		if (Header.cartridgeType == 0x0)
		{
			bank = 1;
			return bank;
		}

		if (m_Register_MBC1_Mode == 0)
		{
			// we only care about the lower 2 bits of the bank 2 register
			// they get appended to bits 5 and 6
			bank |= (m_Register_MBC1_Bank2 & 0b11) << 5;
		}

		// if the rom is larger than 256KiB than we want to keep the 5th bit
		if (Header.romSize > 0x03)
		{
			return bank & 0b11111;
		}
		else
		{
			return bank & 0b1111;
		}
	}

	uint8_t Cartridge::GetRamBank()
	{
		// during mode 1, the ram bank is selected from the 2 lowest address bits
		if (m_Register_MBC1_Mode == 1)
		{
			return m_Register_MBC1_Bank2 & 0b11;
		}

		// during mode 0, only the first ram bank is used
		return 0;
	}
}
