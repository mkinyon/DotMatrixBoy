#pragma once

#include <stdint.h>

#include "Mmu.h"
#include "Defines.h"

namespace Core
{
	class Input
	{
	public:
		Input(Mmu& mmu);
		~Input();

		void Clock();
		void SetDPADState(const Joypad_DPAD button, const bool isPressed);
		void SetButtonState(const Joypad_Button, const bool isPressed);

	private:
		Mmu& mmu;
		uint8_t dpadState = 0xFF;
		uint8_t buttonState = 0xFF;
	};
}