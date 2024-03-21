#pragma once
#include <stdint.h>
#include <random>
#include <sstream>
#

namespace Core
{
	bool GetFlag(uint8_t& variable, int flag);
	void SetFlag(uint8_t& variable, int flag);
	void ClearFlag(uint8_t& variable, int flag);

	int GenerateRandomNumber(int start, int end);

	std::string FormatInt(uint32_t n, uint8_t d);
	std::string FormatHex(uint32_t n, uint8_t d);

	bool EnsureDirectoryExists(const std::string& directory);
	bool CreateSaveFile(std::string fileName, std::vector<uint8_t>* saveData);
	bool LoadSaveFile(std::string fileName, std::vector<uint8_t>* saveData);
	std::string StripPathAndExt(std::string input);
}