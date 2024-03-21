#pragma once
#include <cstdint>

#include "Cartridge.h"
#include "BaseDevice.h"
#include "Defines.h"

namespace Core
{
	class Mmu
	{
	public:
		Mmu(Cartridge& cart);
		~Mmu();

		uint8_t& Read(uint16_t address, const bool hasPPUAccess = false);
		void Write(uint16_t address, uint8_t value, bool rawWrite = false);
		bool ReadRegisterBit(uint16_t address, int flag, const bool hasPPUAccess = false);
		void WriteRegisterBit(uint16_t address, int flag, bool isEnabled);
		void RegisterOnWrite(BaseDevice* device);
		void ResetDIVTimer();

		LCD_Mode ReadLCDMode() const;
		void WriteLCDMode(const LCD_Mode lcdMode);

	private:
		std::vector<uint8_t>* m_Memory;
		std::vector<uint8_t> m_BadReadData = { 0xFF, 0xFF, 0xFF };

		Cartridge m_Cart;
		LCD_Mode m_CurrentLCDMode = LCD_Mode::MODE_0_HBLANK;

		std::vector<BaseDevice*> m_RegisteredDevices;
	};
}