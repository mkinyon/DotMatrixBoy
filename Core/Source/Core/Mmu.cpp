
#include "Mmu.h"
#include "Logger.h"
#include "Utils.h"

namespace Core
{
	Mmu::Mmu(Cartridge& cart) : m_Cart(cart)
	{
		m_Memory = new std::vector<uint8_t>(0xFFFF + 1);
	}

	Mmu::~Mmu() {}

	uint8_t& Mmu::Read(uint16_t address, const bool hasPPUAccess)
	{
		// cart address space
		if (address >= CART_ADDR_RANGE_START && address <= CART_ADDR_RANGE_END)
		{
			return m_Cart.Read(address);
		}
		// ignore reads to VRAM while PPU is drawing
		else if (address >= 0x8000 && address <= 0xFE9F && !hasPPUAccess && m_CurrentLCDMode == LCD_Mode::MODE_3_DRAWING)
		{
			return m_BadReadData.at(0);
		}
		// ignore reads to OAM address space while PPU is drawing or scanning OAM objects
		else if (address >= 0xFE00 && address <= 0xFE9F && !hasPPUAccess &&
			(m_CurrentLCDMode == LCD_Mode::MODE_3_DRAWING || m_CurrentLCDMode == LCD_Mode::MODE_2_OAMSCAN))
		{
			return m_BadReadData.at(0);
		}
		else
		{
			return m_Memory->at(address);
		}
	}

	void Mmu::Write(uint16_t address, uint8_t value, bool rawWrite)
	{	
		if (rawWrite)
		{
			m_Memory->at(address) = value;
			return;
		}

		// we'll let the cartridge handle any writes to that address space
		if (address >= CART_ADDR_RANGE_START && address <= CART_ADDR_RANGE_END)
		{
			m_Cart.Write(address, value);
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

				uint8_t copyValue = Read(copyFrom, true);
				Write(copyTo, copyValue);
			}
		}
		// writes to the LCDY register will reset it
		else if (address == HW_LY_LCD_Y_COORD)
		{
			m_Memory->at(HW_LY_LCD_Y_COORD) = 0;
		}
		// writes to the joypad register will require us to preserve the lower bits
		else if (address == HW_P1JOYP_JOYPAD)
		{
			m_Memory->at(HW_P1JOYP_JOYPAD) = (value & 0x30) | (m_Memory->at(HW_P1JOYP_JOYPAD) & 0xCF);
		}
		// writes to the lcd status register will require us to preserve the lower two bits
		else if (address == HW_STAT_LCD_STATUS)
		{
			m_Memory->at(HW_STAT_LCD_STATUS) = (value & 0x7C) | (m_Memory->at(HW_STAT_LCD_STATUS) & 0x03);
		}
		else
		{
			m_Memory->at(address) = value;
		}	

		for (BaseDevice* device : m_RegisteredDevices)
		{
			device->OnWrite(address, value);
		}
	}

	bool Mmu::ReadRegisterBit(uint16_t address, int flag, const bool hasPPUAccess)
	{
		uint8_t value = Read(address, hasPPUAccess);
		return GetFlag(value, flag);
	}

	void Mmu::WriteRegisterBit(uint16_t address, int flag, bool isEnabled)
	{
		uint8_t value = Read(address);

		if (isEnabled)
		{
			SetFlag(value, flag);
		}	
		else
		{
			ClearFlag(value, flag);
		}

		Write(address, value);
	}

	void Mmu::RegisterOnWrite(BaseDevice* device)
	{
		m_RegisteredDevices.push_back(device);
	}

	void Mmu::ResetDIVTimer()
	{
		m_Memory->at(HW_DIV_DIVIDER_REGISTER) = 0;
		m_Memory->at(HW_DIV_DIVIDER_REGISTER - 1) = 0;
	}

	LCD_Mode Mmu::ReadLCDMode() const
	{
		return m_CurrentLCDMode;
	}

	void Mmu::WriteLCDMode(const LCD_Mode lcdMode)
	{
		uint8_t lcdModeValue = static_cast<uint8_t>(lcdMode);
		WriteRegisterBit(HW_STAT_LCD_STATUS, 0x2, (lcdModeValue >> 1) & 0x1);
		WriteRegisterBit(HW_STAT_LCD_STATUS, 0x1, lcdModeValue & 0x1);
		m_CurrentLCDMode = lcdMode;
	}
}