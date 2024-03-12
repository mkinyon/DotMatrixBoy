#pragma once
#include <cstdint>

#include "Cartridge.h"
#include "BaseDevice.h"

namespace Core
{
	class Mmu
	{
	public:
		Mmu(Cartridge& cart);
		~Mmu();

		uint8_t& Read(uint16_t address);
		void Write(uint16_t address, uint8_t value, bool rawWrite = false);
		bool ReadRegisterBit(uint16_t address, int flag);
		void WriteRegisterBit(uint16_t address, int flag, bool isEnabled);
		void RegisterOnWrite(BaseDevice* device);
		void ResetDIVTimer();

	private:
		uint8_t m_Memory[0xFFFF + 1] = {};
		Cartridge m_Cart;

		std::vector<BaseDevice*> m_RegisteredDevices;
	};
}