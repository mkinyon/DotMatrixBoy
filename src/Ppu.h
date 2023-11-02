#pragma once
#include <stdint.h>

class Ppu
{
public:
	Ppu();
	~Ppu();

public:
	void Clock(GameBoy& gb);

private:
	uint8_t screenPixels[160 * 144];
};

