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
		std::vector<float> GetMasterAudioBuffer();
		std::vector<float> GetCh1AudioBuffer();
		std::vector<float> GetCh2AudioBuffer();

		void SquareWaveTest(uint8_t* stream, int len);
		void FeedAudioBuffer(uint8_t* stream, int len);
		int GetRingBufferSize() const;

	private:
		void OnWrite(uint16_t address, uint8_t value);

	private:
		Mmu& m_MMU;
		SquareChannel m_CH1_Square;
		SquareChannel m_CH2_Square;

		int m_CycleCount = 0;
		uint8_t m_FrameSequencer = 0;
		int m_FrameSequenceCountDown = 8192;

		SDL_AudioDeviceID m_SDLAudioDevice;

		RingBuffer ringBuffer;
		std::vector<float> m_MasterBuffer;
		std::vector<float> m_CH1Buffer;
		std::vector<float> m_CH2Buffer;

		const int SAMPLE_SIZE = AUDIO_SAMPLE_SIZE;
	};
}