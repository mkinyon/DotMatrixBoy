
#include "Apu.h"
#include "..\Defines.h"
#include <stdint.h>
#include "../../../../ThirdParty/SDL2/include/SDL_audio.h"
#include "../../../../ThirdParty/SDL2/include/SDL.h"
#include "..\Logger.h"


namespace Core
{
	Apu::Apu(Mmu& mmu) : mmu(mmu), ch1_square(mmu, true), ch2_square(mmu, false), buffer()
	{
		mmu.RegisterOnWrite(this);

		file.setSampleRate(sampleRate);
		file.setAudioBufferSize(numChannels, 100000);

		// Setup audio
		if (SDL_Init(SDL_INIT_AUDIO) < 0)
		{
			// Handle SDL initialization failure
			return;
		}

		SDL_AudioSpec want, have;
		SDL_memset(&want, 0, sizeof(want));
		want.freq = 44100;
		want.format = AUDIO_F32SYS;
		want.channels = 1;
		want.samples = SAMPLE_SIZE;
		want.callback = NULL;
		want.userdata = this;

		sdlAudioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
		SDL_PauseAudioDevice(sdlAudioDevice, 0);  // Start audio playback

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

		// only read the sample every (cpu frequency / 44100)
		if (cycleCount >= 95)
		{
			file.samples[0].push_back(static_cast<float>(ch1_square.CurrentSample * 10));
			if (file.samples[0].size() == 100000)
			{
				file.save(filename);
			}


			buffer[sampleCounter++] = (static_cast<float>(ch1_square.CurrentSample * 10));

			if (sampleCounter == SAMPLE_SIZE)
			{
				sampleCounter = 0;

				//SDL_QueueAudio(sdlAudioDevice, buffer, sizeof(float) * SAMPLE_SIZE);
				Uint32 queuedBytes = SDL_GetQueuedAudioSize(sdlAudioDevice);
				if (queuedBytes == 0) {
					// The audio queue is empty
					// You may want to enqueue more audio samples
					Logger::Instance().Warning(Core::Domain::APU, "Queue is empty!");
				}
				else
				{
					Logger::Instance().Warning(Core::Domain::APU, "Queue is not empty!");
				}
			}
			cycleCount = 0;
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
		/*if (address == HW_NR11_SOUND_CHANNEL_1_LEN_TIMER)
		{
			ch1_square.lengthLoad =
		}*/
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
		//squareWaveTest(stream, len);

		for (int i = 0; i < len; i += 1)
		{
			//int16_t intSample = static_cast<int16_t>((audioBuffer.Read() * 10) * 32767.0);

			//stream[i] = static_cast<uint8_t>(intSample & 0xFF);
			//stream[i + 1] = static_cast<uint8_t>((intSample >> 8) & 0xFF);  // High byte 

		}
	}
}