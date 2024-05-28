#pragma once

#include "AudioFile.h"
#include "..\Mmu.h"
#include "PulseChannel.h"
#include "NoiseChannel.h"
#include "WaveChannel.h"
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
		std::vector<float> GetCh3AudioBuffer();
		std::vector<float> GetCh4AudioBuffer();
		int GetQueuedAudioBufferSize();

	private:
		void OnWrite(uint16_t address, uint8_t value);
		float NormalizeSample(float sample);

	private:
		Mmu& m_MMU;
		PulseChannel m_CH1_Pulse;
		PulseChannel m_CH2_Pulse;
		WaveChannel m_CH3_Wave;
		NoiseChannel m_CH4_Noise;

		int m_CycleCount = 0;
		uint8_t m_FrameSequencer = 0;
		int m_FrameSequenceCountDown = 8192;

		SDL_AudioDeviceID m_SDLAudioDevice;

		AudioFile<float> m_File;
		std::vector<float> m_MasterBuffer;
		std::vector<float> m_CH1Buffer;
		std::vector<float> m_CH2Buffer;
		std::vector<float> m_CH3Buffer;
		std::vector<float> m_CH4Buffer;

		const int SAMPLE_SIZE = AUDIO_SAMPLE_SIZE;
	};
}