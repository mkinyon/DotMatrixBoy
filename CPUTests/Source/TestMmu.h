
#pragma once
#include <cstdint>
#include <vector>

#include "Core/BaseDevice.h"
#include "Core/Defines.h"
#include "Core/IMmu.h"

class TestMmu : public Core::IMmu
{
public:
	TestMmu();
	~TestMmu();

	uint8_t& Read(uint16_t address, const bool hasPPUAccess = false);
	void Write(uint16_t address, uint8_t value, bool rawWrite = false);

	void Reset();

	bool ReadRegisterBit(uint16_t address, int flag, const bool hasPPUAccess = false) { return false; };
	void WriteRegisterBit(uint16_t address, int flag, bool isEnabled) {};
	void RegisterOnWrite(Core::BaseDevice* device) {};
	void ResetDIVTimer() {};

	Core::LCD_Mode ReadLCDMode() const { return Core::LCD_Mode::MODE_0_HBLANK; };
	void WriteLCDMode(const Core::LCD_Mode lcdMode) {};

private:
	std::vector<uint8_t>* m_Memory;
};
