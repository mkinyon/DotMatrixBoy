#pragma once
#include <stdint.h>

template<typename T>
bool getFlag(uint8_t& variable, T flag)
{
	return (variable & flag) != 0;
}

template<typename T>
void setFlag(uint8_t& variable, T flag)
{
	variable |= flag;
}

template<typename T>
void clearFlag(uint8_t& variable, T flag)
{
	variable &= ~flag;
}
