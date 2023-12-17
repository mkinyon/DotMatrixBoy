#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <cstdint>

namespace Core
{
	class Cartridge
	{
	public:
		Cartridge();
		Cartridge(const std::string& fileName, bool enableBootRom);
		~Cartridge();

		uint8_t& Read(uint16_t address);

		std::vector<uint8_t>* romData;
	};
}

