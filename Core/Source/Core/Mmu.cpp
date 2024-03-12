
#include "Mmu.h"
#include "Logger.h"
#include "Defines.h"
#include "Utils.h"

namespace Core
{
	Mmu::Mmu(Cartridge& cart) : m_Cart(cart) {}
	Mmu::~Mmu() {}

	uint8_t& Mmu::Read(uint16_t address)
	{
		// test to see if read ever go out of bounds
		if (address < 0 || address > 0xFFFF)
		{
			int breakpoint = 0;
		}

		// cart address space
		if (address >= 0x0000 && address <= 0x7FFF)
		{
			return m_Cart.Read(address);
		}

		return m_Memory[address];
	}

	void Mmu::Write(uint16_t address, uint8_t value, bool rawWrite)
	{	
		if (rawWrite)
		{
			m_Memory[address] = value;
			return;
		}

		// test to see if writes ever go out of bounds
		if (address < 0 || address > 0xFFFF)
		{
			int breakpoint = 0;
		}

		if (address >= CART_ADDR_RANGE_START && address <= CART_ADDR_RANGE_END)
		{
			m_Cart.Write(address, value);
		}
		else if (address >= CARTRAM_ADDR_RANGE_START && address <= CARTRAM_ADDR_RANGE_END)
		{
			m_Cart.Write(address, value);
		}
		// $E000-$FDFF   Echo RAM (Not used) (This shouldn't be written to??)
		else if (address >= 0xE000 && address <= 0xFDFF)
		{
			// this should never happen
			std::ostringstream stream;
			stream << "Can't write to echo RAM! Address: " << std::hex << address << " Value: " << std::dec << value;
			Logger::Instance().Info(Domain::MMU, stream.str());
		}
		// writing to the DIV register will cause is to reset to zero 
		else if (address == HW_DIV_DIVIDER_REGISTER)
		{
			ResetDIVTimer();
		}
		// trigger DMA transfer
		else if (address == HW_DMA_OAM_DMA_SOURCE_ADDRESS)
		{
			uint16_t startAddress = value << 8;

			for (int i = 0x0; i <= 0x9F; i++)
			{
				uint16_t copyFrom = startAddress + i;
				uint16_t copyTo = 0xFE00 + i;

				uint8_t copyValue = Read(copyFrom);
				Write(copyTo, copyValue);
			}

			std::ostringstream stream;
			stream << "DMA write. Address: " << std::hex << address << " Value: " << std::dec << value;
			Logger::Instance().Info(Domain::MMU, stream.str());
		}
		else
		{
			m_Memory[address] = value;
		}	

		for (BaseDevice* device : m_RegisteredDevices)
		{
			device->OnWrite(address, value);
		}
	}

	bool Mmu::ReadRegisterBit(uint16_t address, int flag)
	{
		uint8_t value = Read(address);
		return GetFlag(value, flag);
	}

	void Mmu::WriteRegisterBit(uint16_t address, int flag, bool isEnabled)
	{
		uint8_t value = Read(address);

		if (isEnabled)
			SetFlag(value, flag);
		else
			ClearFlag(value, flag);

		Write(address, value);
	}

	void Mmu::RegisterOnWrite(BaseDevice* device)
	{
		m_RegisteredDevices.push_back(device);
	}

	void Mmu::ResetDIVTimer()
	{
		m_Memory[HW_DIV_DIVIDER_REGISTER] = 0;
		m_Memory[HW_DIV_DIVIDER_REGISTER - 1] = 0;
	}
}