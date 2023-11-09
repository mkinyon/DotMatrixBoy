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