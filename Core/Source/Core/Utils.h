#pragma once
#include <stdint.h>
#include <random>
#include <sstream>

namespace Core
{
	bool getFlag(uint8_t& variable, int flag);
	void setFlag(uint8_t& variable, int flag);
	void clearFlag(uint8_t& variable, int flag);

	int generateRandomNumber(int start, int end);

	std::string FormatInt(uint32_t n, uint8_t d);
	std::string FormatHex(uint32_t n, uint8_t d);
}