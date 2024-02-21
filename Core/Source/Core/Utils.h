#pragma once
#include <stdint.h>
#include <random>
#include <sstream>

namespace Core
{
	bool GetFlag(uint8_t& variable, int flag);
	void SetFlag(uint8_t& variable, int flag);
	void ClearFlag(uint8_t& variable, int flag);

	int GenerateRandomNumber(int start, int end);

	std::string FormatInt(uint32_t n, uint8_t d);
	std::string FormatHex(uint32_t n, uint8_t d);
}