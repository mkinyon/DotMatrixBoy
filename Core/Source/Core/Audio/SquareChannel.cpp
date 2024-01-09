
#include "SquareChannel.h"


namespace Core
{
	SquareChannel::SquareChannel(Mmu& mmu, bool isChannel1) : mmu(mmu)
	{
		this->isChannel1 = isChannel1;
	}

	SquareChannel::~SquareChannel() {}

	void SquareChannel::Clock()
	{
		cycleCount++;
		if (cycleCount >= cycleSampleUpdate)
		{
			sampleIndex++;
			if (sampleIndex > 7)
			{
				sampleIndex = 0;
			}

			updateSample();
			cycleCount -= cycleSampleUpdate;
		}
	}

	void SquareChannel::LengthClock()
	{
		uint8_t lengthCounter = mmu.Read(lengthDutyAddr) & 0b111111;
		bool lengthEnable = mmu.ReadRegisterBit(dataAddr, NR14_LEN_ENABLE);

		if (lengthCounter > 0 && lengthEnable)
		{
			lengthCounter--;
			if (lengthCounter == 0)
			{
				isActive = 0;
				mmu.WriteRegisterBit(HW_NR52_SOUND_TOGGLE, soundControlFlag, false);
			}
		}
	}

	void SquareChannel::SweepClock()
	{

	}

	void SquareChannel::EnvelopeClock()
	{

	}

	void SquareChannel::Trigger()
	{
		isActive = 1;
		mmu.WriteRegisterBit(HW_NR52_SOUND_TOGGLE, soundControlFlag, true);

		selectedDuty = (mmu.Read(lengthDutyAddr) & 0b11000000) >> 6;

		// https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Square_Wave
		// a square channel's frequency timer period is set to (2048-frequency)*4
		currentFrequency = getFrequency();
		cycleSampleUpdate = (2048 - currentFrequency) * 4;
		cycleCount = 0;
		sampleIndex = 0;
	}

	uint16_t SquareChannel::getFrequency()
	{
		uint8_t frequencyData = mmu.Read(dataAddr);
		uint16_t frequency = mmu.Read(freqLowAddr);
		frequency |= (frequencyData & 0b111) << 8;
		return frequency;
	}

	void SquareChannel::updateSample()
	{
		uint8_t dutyValue = dutyCycleTable[selectedDuty][sampleIndex];
		CurrentSample = dutyValue;

		if (!isActive)
		{
			CurrentSample = 0;
		}
	}
}