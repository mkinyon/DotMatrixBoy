
#include "Input.h"

namespace Core
{
	Input::Input(Mmu& mmu) : mmu(mmu) {}
	Input::~Input() {}

	void Input::Clock()
	{
		bool stateChanged = false;
		uint8_t newState = 0;

		// if the dpad flag is not currently being read then its ok to update the dpad states
		if (!mmu.ReadFromMemoryMapRegister(HW_P1JOYP_JOYPAD, JOYP_SELECT_DPAD))
		{
			newState = dpadState & 0x0F;
			stateChanged = (mmu.ReadFromMemoryMap(HW_P1JOYP_JOYPAD) & 0x0F) ^ newState;
		}
		// if the buttons flag is not currently being read then its ok to update the button states
		else if (!mmu.ReadFromMemoryMapRegister(HW_P1JOYP_JOYPAD, JOYP_SELECT_BUTTONS))
		{
			newState = buttonState & 0x0F;
			stateChanged = (mmu.ReadFromMemoryMap(HW_P1JOYP_JOYPAD) & 0x0F) ^ newState;
		}

		if (stateChanged)
		{
			uint8_t currentState = mmu.ReadFromMemoryMap(HW_P1JOYP_JOYPAD);

			// We only want to update the lower 4 bits of the JOYPAD register which 
			// contain the buttons that are pressed.
			mmu.WriteToMemoryMap(HW_P1JOYP_JOYPAD, (currentState & 0xF0) | newState);
			mmu.WriteToMemoryMapRegister(HW_IF_INTERRUPT_FLAG, IF_JOYPAD, true);
		}
	}

	void Input::SetDPADState(const Joypad_DPAD button, const bool isPressed)
	{
		if (isPressed)
		{
			dpadState &= ~button;
		}
		else
		{
			dpadState |= button;
		}
	}

	void Input::SetButtonState(const Joypad_Button button, const bool isPressed)
	{
		if (isPressed)
		{
			buttonState &= ~button;
		}
		else
		{
			buttonState |= button;
		}
	}
}