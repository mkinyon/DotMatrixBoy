
#include "Apu.h"
#include "..\Defines.h"
#include <stdint.h>
#include "../../../../ThirdParty/SDL2/include/SDL_audio.h"
#include "../../../../ThirdParty/SDL2/include/SDL.h"
#include "..\Logger.h"


namespace Core
{
	Apu::Apu(Mmu& mmu) :
		m_MMU(mmu), 
		m_CH1_Square(mmu, true), 
		m_CH2_Square(mmu, false), 
		m_MasterBuffer(),
		m_CH1Buffer(),
		m_CH2Buffer()
	{
		mmu.RegisterOnWrite(this);

		m_File.setSampleRate(m_SampleRate);
		m_File.setAudioBufferSize(m_NumChannels, 100000);

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

		m_SDLAudioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
		SDL_PauseAudioDevice(m_SDLAudioDevice, 0);  // Start audio playback

	}

	Apu::~Apu() {}

	void Apu::Clock()
	{
		m_CycleCount++;

		if (!m_MMU.ReadRegisterBit(HW_NR52_SOUND_TOGGLE, NR52_AUDIO_ON))
		{
			return;
		}

		m_FrameSequenceCountDown--;
		if (m_FrameSequenceCountDown <= 0)
		{
			m_FrameSequenceCountDown = 8192;
			switch (m_FrameSequencer)
			{
				case 0:
					m_CH1_Square.LengthClock();
					m_CH2_Square.LengthClock();
					break;
				case 1:
					break;
				case 2:
					m_CH1_Square.SweepClock();
					m_CH1_Square.LengthClock();
					m_CH2_Square.LengthClock();
					break;
				case 3:
					break;
				case 4:
					m_CH1_Square.LengthClock();
					m_CH2_Square.LengthClock();
					break;
				case 5:
					break;
				case 6:
					m_CH1_Square.SweepClock();
					m_CH1_Square.LengthClock();
					m_CH2_Square.LengthClock();
					break;
				case 7: 
					m_CH1_Square.EnvelopeClock();
					m_CH2_Square.EnvelopeClock();
					break;
			}

			m_FrameSequencer++;
			if (m_FrameSequencer >= 8)
			{
				m_FrameSequencer = 0;
			}
		}

		m_CH1_Square.Clock();
		m_CH2_Square.Clock();

		// only read the sample every (cpu frequency / 44100)
		if (m_CycleCount >= 95)
		{
			/*m_File.samples[0].push_back(static_cast<float>(m_CH1_Square.m_CurrentSample * 10));
			if (m_File.samples[0].size() == 100000)
			{
				m_File.save(m_Filename);
			}*/

			// check to see if audio channels are enabled
			bool isCh1On = m_MMU.ReadRegisterBit(HW_NR52_SOUND_TOGGLE, NR52_CH1_ON);
			bool isCh2On = m_MMU.ReadRegisterBit(HW_NR52_SOUND_TOGGLE, NR52_CH2_ON);

			// get sample from each channel
			float ch1Sample = isCh1On ? static_cast<float>(m_CH1_Square.m_CurrentSample * 1) : 0;
			float ch2Sample = isCh2On ? static_cast<float>(m_CH2_Square.m_CurrentSample * 1) : 0;

			// add samples to audio buffers
			m_CH1Buffer[m_SampleCounter] = ch1Sample;
			m_CH2Buffer[m_SampleCounter] = ch2Sample;
			m_MasterBuffer[m_SampleCounter] = ch1Sample + ch2Sample;

			m_SampleCounter++;

			if (m_SampleCounter == SAMPLE_SIZE)
			{
				m_SampleCounter = 0;
				
				Uint32 queuedBytes = SDL_GetQueuedAudioSize(m_SDLAudioDevice);
				if (queuedBytes == 0) {
					// The audio queue is empty
					// You may want to enqueue more audio samples
					Logger::Instance().Warning(Core::Domain::APU, "Queue is empty!");
				}
				else
				{
					std::ostringstream stream;
					stream << "Audio Queued Bytes: " << queuedBytes;
					Logger::Instance().Info(Core::Domain::APU, stream.str());
				}

				SDL_QueueAudio(m_SDLAudioDevice, m_MasterBuffer, sizeof(float) * SAMPLE_SIZE);
			}
			m_CycleCount = 0;
		}
	}

	void Apu::OnWrite(uint16_t address, uint8_t value)
	{
		if (address == HW_NR14_SOUND_CHANNEL_1_PERIOD_HIGH)
		{
			if (m_MMU.ReadRegisterBit(HW_NR14_SOUND_CHANNEL_1_PERIOD_HIGH, NR14_TRIGGER))
			{
				m_CH1_Square.Trigger();
			}
		}
		if (address == HW_NR24_SOUND_CHANNEL_2_PERIOD_HIGH)
		{
			if (m_MMU.ReadRegisterBit(HW_NR24_SOUND_CHANNEL_2_PERIOD_HIGH, NR14_TRIGGER))
			{
				m_CH2_Square.Trigger();
			}
		}
		/*if (address == HW_NR11_SOUND_CHANNEL_1_LEN_TIMER)
		{
			m_CH1_Square.lengthLoad =
		}*/
	}

	void Apu::SquareWaveTest(uint8_t* stream, int len)
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
		SquareWaveTest(stream, len);

		for (int i = 0; i < len; i += 1)
		{
			//int16_t intSample = static_cast<int16_t>((audioBuffer.Read() * 10) * 32767.0);

			//stream[i] = static_cast<uint8_t>(intSample & 0xFF);
			//stream[i + 1] = static_cast<uint8_t>((intSample >> 8) & 0xFF);  // High byte 

		}
	}

	float* Apu::GetMasterAudioBuffer()
	{
		return m_MasterBuffer;
	}

	float* Apu::GetCh1AudioBuffer()
	{
		return m_CH1Buffer;
	}

	float* Apu::GetCh2AudioBuffer()
	{
		return m_CH2Buffer;
	}
}