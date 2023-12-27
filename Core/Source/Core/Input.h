#pragma once

#include <stdint.h>
#include "Defines.h"

namespace Core
{
	// forward decl
	class GameBoy;

	class Input
	{
	public:
		Input(GameBoy& gb);
		~Input();

		void Clock();
		void SetDPADState(const Joypad_DPAD button, const bool isPressed);
		void SetButtonState(const Joypad_Button, const bool isPressed);

	private:
		GameBoy& gameboy;
		uint8_t dpadState = 0xFF;
		uint8_t buttonState = 0xFF;
	};
}