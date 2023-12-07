#pragma once
#include <stdint.h>
#include <random>

bool getFlag(uint8_t& variable, int flag);
void setFlag(uint8_t& variable, int flag);
void clearFlag(uint8_t& variable, int flag);

int generateRandomNumber(int start, int end);
