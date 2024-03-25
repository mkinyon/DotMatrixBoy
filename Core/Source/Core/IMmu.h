
#pragma once
#include <cstdint>

#include "BaseDevice.h"
#include "Defines.h"

namespace Core
{
	class IMmu
	{
	public:
		IMmu() {}
		virtual ~IMmu() {}

		virtual uint8_t& Read(uint16_t address, const bool hasPPUAccess = false) = 0;
		virtual void Write(uint16_t address, uint8_t value, bool rawWrite = false) = 0;

		virtual bool ReadRegisterBit(uint16_t address, int flag, const bool hasPPUAccess = false) = 0;
		virtual void WriteRegisterBit(uint16_t address, int flag, bool isEnabled) = 0;
		virtual void RegisterOnWrite(BaseDevice* device) = 0;
		virtual void ResetDIVTimer() = 0;

		virtual LCD_Mode ReadLCDMode() const = 0;
		virtual void WriteLCDMode(const LCD_Mode lcdMode) = 0;
	};
}