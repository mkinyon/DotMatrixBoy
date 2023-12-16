#include "Utils.h"

bool getFlag(uint8_t& variable, int flag)
{
	return (variable & flag) != 0;
}

void setFlag(uint8_t& variable, int flag)
{
	variable |= flag;
}

void clearFlag(uint8_t& variable, int flag)
{
	variable &= ~flag;
}

int generateRandomNumber(int start, int end)
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