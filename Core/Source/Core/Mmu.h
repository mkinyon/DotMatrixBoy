#pragma once
#include <cstdint>

#include "Cartridge.h"

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

	private:
		uint8_t memory[0xFFFF + 1] = {};
		Cartridge cart;

	};
}