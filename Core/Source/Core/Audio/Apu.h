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
		void SquareWaveTest(uint8_t* stream, int len);

	private:
		Mmu& m_MMU;
		SquareChannel m_CH1_Square;
		SquareChannel m_CH2_Square;

		int m_CycleCount = 0;
		//RingBuffer audioBuffer;
		uint8_t m_FrameSequencer = 0;
		int m_FrameSequenceCountDown = 8192;

		AudioFile<float> m_File;

		const std::string m_Filename = "output.wav";
		const int m_SampleRate = 44100; // Adjust as needed
		const int m_NumChannels = 1;    // Adjust as needed (1 for mono, 2 for stereo, etc.)

		SDL_AudioDeviceID m_SDLAudioDevice;
		float m_Buffer[4096] = { 0 };
		int m_SampleCounter = 0;
		const int SAMPLE_SIZE = 4096;
	};
}