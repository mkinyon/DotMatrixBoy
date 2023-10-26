#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <cstdint>

class Cartridge
{
public:
	Cartridge();
	Cartridge(const std::string& fileName);
	~Cartridge();

	uint8_t& read(uint16_t address );

	std::vector<uint8_t> *romData;
};

