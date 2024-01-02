#include "GameBoy.h"
#include "Logger.h"

namespace Core
{
	GameBoy::GameBoy() : cpu(*this), ppu(*this), apu(*this), input(*this) {}
	GameBoy::~GameBoy() {}

	void GameBoy::Run(bool enableBootRom)
	{
		cpu.Reset(enableBootRom);
	}

	void GameBoy::Clock(float elapsedTimeMS)
	{
		if (!m_isPaused)
		{
			float cyclesToRun = elapsedTimeMS * Core::CYCLES_PER_MS;

			for (int i = 0; i < cyclesToRun; i++)
			{
				cpu.Clock();

				for (int i = 0; i < 4; i++)
				{
					ppu.Clock();
				}

				input.Clock();
			}
		}
	}

	void GameBoy::Pause()
	{
		m_isPaused = true;
	}

	void GameBoy::Unpause()
	{
		m_isPaused = false;
	}

	bool GameBoy::IsPaused()
	{
		return m_isPaused;
	}

	void GameBoy::StepCPU()
	{
		do
		{
			cpu.Clock();

			for (int i = 0; i < 4; i++)
			{
				ppu.Clock();
			}
		} while (!cpu.m_InstructionCompleted);
	}

	void GameBoy::InsertCartridge(Cartridge & cartridge)
	{
		cart = cartridge;
	}

	uint8_t & GameBoy::ReadFromMemoryMap(uint16_t address)
	{
		// $0000-$7FFF   RES and Interrupts, cart header, cart data
		if (address >= 0x0000 && address <= 0x7FFF)
		{
			return cart.Read(address);
		}
		// $8000-$97FF   Character RAM
		else if (address >= 0x8000 && address <= 0x97FF)
		{
			uint16_t offset = address - 0x8000;
			return characterRam[offset];
		}
		// $9800-$9BFF   BG Map Data 1
		else if (address >= 0x9800 && address <= 0x9BFF)
		{
			uint16_t offset = address - 0x9800;
			return bgMapData1[offset];
		}
		// $9C00-$9FFF   BG Map Data 2
		else if (address >= 0x9C00 && address <= 0x9FFF)
		{
			uint16_t offset = address - 0x9C00;
			return bgMapData2[offset];
		}
		// $A000-$BFFF   Cartridge RAM (If available)
		else if (address >= 0xA000 && address <= 0xBFFF)
		{
			uint16_t offset = address - 0xA000;
			return cartRam[offset];
		}
		// $C000-$DFFF   Internal RAM (Bank 0 and Banks 1-7 switchable for CGB)
		else if (address >= 0xC000 && address <= 0xDFFF)
		{
			uint16_t offset = address - 0xC000;
			return ram[offset];
		}
		// $E000-$FDFF   Echo RAM (this is just a copy of regular ram so we point there)
		else if (address >= 0xE000 && address <= 0xFDFF)
		{
			uint16_t offset = address - 0xE000;
			return ram[offset];
		}
		// $FE00-$FE9F   OAM - Object Attribute Memory
		else if (address >= 0xFE00 && address <= 0xFE9F)
		{
			uint16_t offset = address - 0xFE00;
			return oam[offset];
		}
		// $FF00-$FF7F   Hardware IO
		else if (address >= 0xFF00 && address <= 0xFF7F)
		{
			uint16_t offset = address - 0xFF00;
			return hardwareIO[offset];
		}
		// $FF80-$FFFE   Zero Page
		else if (address >= 0xFF80 && address <= 0xFFFE)
		{
			uint16_t offset = address - 0xFF80;
			return zeroPage[offset];
		}
		// $FFFF         Interrupt Enable Register
		else if (address == 0xFFFF)
		{
			return interruptEnable;
		}
		else
		{
			std::ostringstream stream;
			stream << "Bad Memory Map write! Address: " << std::hex << address;
			Logger::Instance().Error(Domain::MMU, stream.str());
		}
	}

	void GameBoy::WriteToMemoryMap(uint16_t address, uint8_t value, bool rawWrite)
	{
		// $0000-$7FFF   RES and Interrupts, cart header, cart data
		if (address >= 0x0000 && address <= 0x7FFF)
		{
			if (address >= 0x2000 && address <= 0x3FFF)
			{
				std::ostringstream stream;
				stream << "Bank Number Write Detected. Address: " << std::hex << address << " Value: " << std::dec << value;
				Logger::Instance().Info(Domain::MMU, stream.str());
				cart.CurrentBankNumber = value & 0xF;
			}
			else
			{
				std::ostringstream stream;
				stream << "Unmapped cart address! Address: " << std::hex << address << " Value: " << std::dec << value;
				Logger::Instance().Warning(Domain::MMU, stream.str());
			}
		}
		// $8000-$97FF   Character RAM
		else if (address >= 0x8000 && address <= 0x97FF)
		{
			uint16_t offset = address - 0x8000;
			characterRam[offset] = value;
		}
		// $9800-$9BFF   BG Map Data 1
		else if (address >= 0x9800 && address <= 0x9BFF)
		{
			uint16_t offset = address - 0x9800;
			bgMapData1[offset] = value;
		}
		// $9C00-$9FFF   BG Map Data 2
		else if (address >= 0x9C00 && address <= 0x9FFF)
		{
			uint16_t offset = address - 0x9C00;
			bgMapData2[offset] = value;
		}
		// $A000-$BFFF   Cartridge RAM (If available)
		else if (address >= 0xA000 && address <= 0xBFFF)
		{
			uint16_t offset = address - 0xA000;
			cartRam[offset] = value;
		}
		// $C000-$DFFF   Internal RAM (Bank 0 and Banks 1-7 switchable for CGB)
		else if (address >= 0xC000 && address <= 0xDFFF)
		{
			uint16_t offset = address - 0xC000;
			ram[offset] = value;
		}
		// $E000-$FDFF   Echo RAM (Not used) (This shouldn't be written to??)
		else if (address >= 0xE000 && address <= 0xFDFF)
		{
			// this should never happen
			std::ostringstream stream;
			stream << "Can't write to echo RAM! Address: " << std::hex << address << " Value: " << std::dec << value;
			Logger::Instance().Info(Domain::MMU, stream.str());
		}
		// $FE00-$FE9F   OAM - Object Attribute Memory
		else if (address >= 0xFE00 && address <= 0xFE9F)
		{
			uint16_t offset = address - 0xFE00;
			oam[offset] = value;
		}
		// $FF00-$FF7F   Hardware IO
		else if (address >= 0xFF00 && address <= 0xFF7F)
		{
			uint16_t offset = address - 0xFF00;

			// writing to the DIV register will cause is to reset to zero 
			if (address == HW_DIV_DIVIDER_REGISTER && !rawWrite)
			{
				hardwareIO[offset] = 0;
				hardwareIO[offset - 1] = 0;
			}

			// trigger DMA transfer
			if (address == HW_DMA_OAM_DMA_SOURCE_ADDRESS)
			{
				hardwareIO[offset] = value;
				uint16_t startAddress = value << 8;

				for (int i = 0x0; i <= 0x9F; i++)
				{
					uint16_t copyFrom = startAddress + i;
					uint16_t copyTo = 0xFE00 + i;

					uint8_t copyValue = ReadFromMemoryMap(copyFrom);
					WriteToMemoryMap(copyTo, copyValue);
				}

				std::ostringstream stream;
				stream << "DMA write. Address: " << std::hex << address << " Value: " << std::dec << value;
				Logger::Instance().Info(Domain::MMU, stream.str());
			}
			
			hardwareIO[offset] = value;
		}
		// $FF80-$FFFE   Zero Page
		else if (address >= 0xFF80 && address <= 0xFFFE)
		{
			uint16_t offset = address - 0xFF80;
			zeroPage[offset] = value;
		}
		// $FFFF         Interrupt Enable Register
		else if (address == 0xFFFF)
		{
			interruptEnable = value;
		}
		else
		{
			std::ostringstream stream;
			stream << "Bad memory map write! Address: " << std::hex << address << " Value: " << std::dec << value;
			Logger::Instance().Error(Domain::PPU, stream.str());
		}
	}

	bool GameBoy::ReadFromMemoryMapRegister(uint16_t address, int flag)
	{
		uint8_t value = ReadFromMemoryMap(address);
		return getFlag(value, flag);
	}

	void GameBoy::WriteToMemoryMapRegister(uint16_t address, int flag, bool isEnabled)
	{
		uint8_t value = ReadFromMemoryMap(address);

		if (isEnabled)
			setFlag(value, flag);
		else
			clearFlag(value, flag);

		WriteToMemoryMap(address, value);
	}

}