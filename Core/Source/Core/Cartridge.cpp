#include "Cartridge.h"
#include "Defines.h"
#include "Logger.h"
#include <ostream>

namespace Core
{
	Cartridge::Cartridge() : m_Header(), m_BootRomData(), m_RomData(), m_RamData() {}

	Cartridge::Cartridge(const std::string& fileName, bool enableBootRom)
	{
		// load rom file into rom data
		std::ifstream file;
		file.open(fileName, std::ifstream::binary);

		if (file.is_open())
		{
			// read header
			file.seekg(0x100); // the header starts at location $100
			file.read((char*)&m_Header, sizeof(sRomHeader));

			file.seekg(0, file.end);
			int cartSize = (unsigned long)file.tellg();

			file.seekg(0); // reset to beginning
			m_RomData = new std::vector<uint8_t>(cartSize);
			file.read((char*)m_RomData->data(), cartSize);

			file.close();
		}

		// load bootrom file into bootrom data
		if (enableBootRom)
		{
			std::ifstream bootrom;
			bootrom.open("../Roms/dmg_boot.bin", std::ifstream::binary);
			if (bootrom.is_open())
			{
				bootrom.seekg(0);

				m_BootRomData = new std::vector<uint8_t>(256);
				bootrom.read((char*)m_BootRomData->data(), 256);

				m_BootRomActive = true;
			}
			else
			{
				// bootrom not found 
				m_BootRomActive = false;
			}

			bootrom.close();
		}

		m_RamData = new std::vector<uint8_t>(32768);
	}

	Cartridge::~Cartridge() {}

	uint8_t& Cartridge::Read(uint16_t address)
	{
		if (address >= CARTBANK_ADDR_RANGE_START && address <= CARTBANK_ADDR_RANGE_END)
		{
			// offset address based on current rombank
			int test = GetRomBank();
			return m_RomData->at(address + ((GetRomBank() - 1) * ROM_BANK_SIZE));
		}
		else if (address >= CARTRAM_ADDR_RANGE_START && address <= CARTRAM_ADDR_RANGE_END)
		{
			if (m_RamEnabled)
			{
				uint16_t ramAddress = address + (GetRomBank() * RAM_BANK_SIZE);
				uint16_t addrOffset = ramAddress - 0xA000;
				return m_RamData->at(addrOffset);
			}
			else
			{
				// if ram is disabled then 0xFF is returned
				return m_BadRamRead;
			}
		}
		else
		{
			// The boot rom is only temporarily mapped until control is handed off to
			// the cartridge program so if the boot rom is active we want to map reads
			// to that.
			if (m_BootRomActive)
			{
				// Address $0100 is the starting address for the catridge program. 
				// At this point we want to disable the bootrom and hand over control.
				if (address == 0x100)
				{
					m_BootRomActive = false;
					return m_RomData->at(address);
				}
				// In some cases the bootrom will read an address from outside the 
				// bootrom address space. We need to allow it because the bootrom
				// needs to access the cartridge program for the logo graphics.
				else if (address > 0x100)
				{
					return m_RomData->at(address);
				}

				return m_BootRomData->at(address);
			}
			else
			{
				return m_RomData->at(address);
			}
		}
	}

	void Cartridge::Write(uint16_t address, uint8_t value)
	{
		// ram bank
		if (address >= MBC1_RAMG_START && address <= MBC1_RAMG_END)
		{
			// Writing 0xA (0b1010) to this address range will enable ram. All
			// other writes will disable it.
			m_RamEnabled = ((value & 0xF) == 0xA);
		}
		else if (address >= MBC1_BANKREG1_START && address <= MBC1_BANKREG1_END)
		{
			uint8_t regValue = value & 0x1F;
			m_Register_MBC1_Bank1 = (regValue == 0x0 ? 0x1 : regValue);
		}
		else if (address >= MBC1_BANKREG2_START && address <= MBC1_BANKREG2_END)
		{
			m_Register_MBC1_Bank2 = value & 0x3;
		}
		else if (address >= MBC1_MODE_START && address <= MBC1_MODE_END)
		{
			m_Register_MBC1_Mode = value & 0x1;
		}
		else if (address >= CARTRAM_ADDR_RANGE_START && address <= CARTRAM_ADDR_RANGE_END)
		{
			if (m_RamEnabled)
			{
				uint16_t ramAddress = address + (GetRamBank() * RAM_BANK_SIZE);
				uint16_t addrOffset = ramAddress - 0xA000;
				m_RamData->at(addrOffset) = value;
			}
		}
	}

	std::vector<uint8_t>* Cartridge::GetRomData() const
	{
		return m_RomData;
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
		if (m_Header.cartridgeType == 0x0)
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
		if (m_Header.romSize > 0x03)
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

	Cartridge::sRomHeader Cartridge::GetRomInfo() const
	{
		return m_Header;
	}
}
