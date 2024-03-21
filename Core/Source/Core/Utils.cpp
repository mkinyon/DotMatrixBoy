
#include "Utils.h"
#include "Defines.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <Windows.h>

namespace Core
{
	bool GetFlag(uint8_t& variable, int flag)
	{
		return (variable & flag) != 0;
	}

	void SetFlag(uint8_t& variable, int flag)
	{
		variable |= flag;
	}

	void ClearFlag(uint8_t& variable, int flag)
	{
		variable &= ~flag;
	}

	int GenerateRandomNumber(int start, int end)
	{
		std::random_device rd;
		std::mt19937 gen(rd());

		std::uniform_int_distribution<int> distribution(0, 255);

		return distribution(gen);
	}

	std::string FormatInt(uint32_t n, uint8_t d)
	{
		std::stringstream s;
		s << n;
		return s.str();
	}

	std::string FormatHex(uint32_t n, uint8_t d)
	{
		std::string s(d, '0');
		for (int i = d - 1; i >= 0; i--, n >>= 4)
			s[i] = "0123456789ABCDEF"[n & 0xF];
		return s;
	};

	bool EnsureDirectoryExists(const std::string& directory)
	{
		std::filesystem::path path(directory);
		if (!std::filesystem::exists(path))
		{
			try
			{
				return std::filesystem::create_directories(path);
			}
			catch (const std::exception& ex)
			{
				std::cerr << "Error creating directory: " << ex.what() << std::endl;
				return false;
			}
		}

		return true;
	}

	bool CreateSaveFile(std::string fileName, std::vector<uint8_t>* saveData)
	{
		std::string cleanFileName = StripPathAndExt(fileName) + ".SAV";

		std::filesystem::path path("Saves");
		std::filesystem::path fullPath = path / cleanFileName;
		std::ofstream outputFile(fullPath, std::ios::binary);

		if (outputFile.is_open())
		{
			outputFile.write(reinterpret_cast<const char*>(saveData->data()), saveData->size());
			outputFile.close();

			return true;
		}

		return false;
	}

	bool LoadSaveFile(std::string fileName, std::vector<uint8_t>* saveData)
	{
		std::string cleanFileName = StripPathAndExt(fileName) + ".SAV";

		std::filesystem::path path("Saves");
		std::filesystem::path fullPath = path / cleanFileName;
		std::ifstream inputFile(fullPath, std::ios::binary);

		if (inputFile.is_open())
		{
			inputFile.seekg(0, inputFile.end);
			int saveDataSize = (unsigned long)inputFile.tellg();
			if (saveDataSize != RAM_SIZE)
			{
				// save data is not the right size
				return false;
			}

			inputFile.seekg(0); // reset to beginning
			inputFile.read((char*)saveData->data(), saveDataSize);
			inputFile.close();

			return true;
		}

		return false;
	}

	std::string StripPathAndExt(std::string input)
	{
		// remove path
		std::filesystem::path path;
		path = input.substr(input.find_last_of("/\\") + 1);

		// remove ext
		path = path.stem();

		return path.string();
	}
}