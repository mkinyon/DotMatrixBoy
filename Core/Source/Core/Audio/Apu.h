#pragma once

#include "..\Mmu.h"
#include "SquareChannel.h"
#include "AudioFile.h"
#include "RingBuffer.h"
#include "../../../../ThirdParty/SDL2/include/SDL_audio.h"

namespace Core
{
	class Apu : BaseDevice
	{
	public:
		Apu(Mmu& mmu);
		~Apu();

		void Clock();
		void FeedAudioBuffer(uint8_t* stream, int len);

	private:
		void OnWrite(uint16_t address, uint8_t value);
		void squareWaveTest(uint8_t* stream, int len);

	private:
		Mmu& mmu;
		SquareChannel ch1_square;
		SquareChannel ch2_square;

		int cycleCount = 0;
		//RingBuffer audioBuffer;
		uint8_t frameSequencer = 0;
		int frameSequenceCountDown = 8192;

		AudioFile<float> file;

		const std::string filename = "output.wav";
		const int sampleRate = 44100; // Adjust as needed
		const int numChannels = 1;    // Adjust as needed (1 for mono, 2 for stereo, etc.)

		SDL_AudioDeviceID sdlAudioDevice;
		float buffer[4096] = { 0 };
		int sampleCounter = 0;
		const int SAMPLE_SIZE = 4096;
	};
}