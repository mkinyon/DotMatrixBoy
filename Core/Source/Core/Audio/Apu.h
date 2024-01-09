#pragma once

#include "..\Mmu.h"
#include "SquareChannel.h"
#include "AudioFile.h"

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
		uint16_t sampleCounter = 0;
		float audioBuffer[1024] = { 0 };
		uint8_t frameSequencer = 0;
		int frameSequenceCountDown = 8192;

		AudioFile<float> file;

		const std::string filename = "output.wav";
		const int sampleRate = 44100; // Adjust as needed
		const int numChannels = 1;    // Adjust as needed (1 for mono, 2 for stereo, etc.)


	};
}