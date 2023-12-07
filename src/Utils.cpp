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