
#include "Apu.h"
#include "..\Defines.h"
#include <stdint.h>


namespace Core
{
	Apu::Apu(Mmu& mmu) : mmu(mmu), ch1_square(mmu, true), ch2_square(mmu, false)
	{
		mmu.RegisterOnWrite(this);

		file.setSampleRate(sampleRate);
		file.setAudioBufferSize(numChannels, 100000);
	}

	Apu::~Apu() {}

	void Apu::Clock()
	{
		cycleCount++;

		if (!mmu.ReadRegisterBit(HW_NR52_SOUND_TOGGLE, NR52_AUDIO_ON))
		{
			return;
		}

		frameSequenceCountDown--;
		if (frameSequenceCountDown <= 0)
		{
			frameSequenceCountDown = 8192;
			switch (frameSequencer)
			{
				case 0:
					ch1_square.LengthClock();
					ch2_square.LengthClock();
					break;
				case 1:
					break;
				case 2:
					ch1_square.SweepClock();
					ch1_square.LengthClock();
					ch2_square.LengthClock();
					break;
				case 3:
					break;
				case 4:
					ch1_square.LengthClock();
					ch2_square.LengthClock();
					break;
				case 5:
					break;
				case 6:
					ch1_square.SweepClock();
					ch1_square.LengthClock();
					ch2_square.LengthClock();
					break;
				case 7: 
					ch1_square.EnvelopeClock();
					ch2_square.EnvelopeClock();
					break;
			}

			frameSequencer++;
			if (frameSequencer >= 8)
			{
				frameSequencer = 0;
			}
		}

		ch1_square.Clock();
		ch2_square.Clock();

		if (cycleCount >= 95)
		{
			audioBuffer[sampleCounter] = static_cast<float>(ch1_square.CurrentSample + ch2_square.CurrentSample) / 2;
			/*file.samples[0].push_back(ch1_square.CurrentSample);
			if (file.samples[0].size() == 100000)
			{
				file.save(filename);
			}*/

			sampleCounter++;
			if (sampleCounter == 1024)
			{
				sampleCounter = 0;
			}
			cycleCount -= 95;
		}
	}

	void Apu::OnWrite(uint16_t address, uint8_t value)
	{
		if (address == HW_NR14_SOUND_CHANNEL_1_PERIOD_HIGH)
		{
			if (mmu.ReadRegisterBit(HW_NR14_SOUND_CHANNEL_1_PERIOD_HIGH, NR14_TRIGGER))
			{
				ch1_square.Trigger();
			}
		}
		if (address == HW_NR24_SOUND_CHANNEL_2_PERIOD_HIGH)
		{
			if (mmu.ReadRegisterBit(HW_NR24_SOUND_CHANNEL_2_PERIOD_HIGH, NR14_TRIGGER))
			{
				ch2_square.Trigger();
			}
		}
	}

	void Apu::squareWaveTest(uint8_t* stream, int len)
	{
		// generating a square wave
		const int sampleRate = 44100;
		static double time = 0.0;
		const double freq = 440; // static_cast<double>(mmu.Read(HW_NR12_SOUND_CHANNEL_1_VOL_ENVELOPE));
		const double amplitude = 10;


		for (int i = 0; i < len; ++i)
		{
			// Toggle between two amplitude values
			double sample = (time < 0.5 / freq) ? amplitude : -amplitude;

			int16_t intSample = static_cast<int16_t>(sample * 32767.0);  // Convert to signed 16-bit

			if (true)
			{
				stream[i] = static_cast<uint8_t>(intSample & 0xFF);            // Low byte
				stream[i + 1] = static_cast<uint8_t>((intSample >> 8) & 0xFF);  // High byte 
			}
			else
			{
				stream[i] = 0;
				stream[i + 1] = 0;
			}

			time += 1.0 / sampleRate;
			if (time >= 1.0 / freq) {
				time -= 1.0 / freq;
			}
		}
	}

	void Apu::FeedAudioBuffer(uint8_t * stream, int len)
	{
		squareWaveTest(stream, len);

		for (int i = 0; i < len; ++i)
		{
			int16_t intSample = static_cast<int16_t>((audioBuffer[i] * 10) * 32767.0);
			stream[i] = static_cast<uint8_t>(intSample & 0xFF);
			stream[i + 1] = static_cast<uint8_t>((intSample >> 8) & 0xFF);  // High byte 
		}
	}
}