
#include "Apu.h"
#include "../Defines.h"
#include <stdint.h>
#include "../../../../ThirdParty/SDL2/include/SDL_audio.h"
#include "../../../../ThirdParty/SDL2/include/SDL.h"
#include "../Logger.h"


namespace Core
{
	Apu::Apu(Mmu& mmu) :
		m_MMU(mmu), 
		m_CH1_Square(mmu, true), 
		m_CH2_Square(mmu, false), 
		m_CH3_Wave(mmu),
		m_CH4_Noise(mmu),
		m_MasterBuffer(),
		m_CH1Buffer(),
		m_CH2Buffer(),
		m_CH3Buffer(),
		m_CH4Buffer(),
		m_SDLAudioDevice()
	{
		mmu.RegisterOnWrite(this);

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
		want.channels = 2;
		want.samples = 1024;

		//SDL_setenv("SDL_AUDIODRIVER", "directsound", 1);
		//SDL_setenv("SDL_AUDIODRIVER", "disk", 1);
		SDL_Init(SDL_INIT_AUDIO);

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
					m_CH3_Wave.LengthClock();
					m_CH4_Noise.LengthClock();
					break;
				case 1:
					break;
				case 2:
					m_CH1_Square.SweepClock();
					m_CH1_Square.LengthClock();
					m_CH2_Square.LengthClock();
					m_CH3_Wave.LengthClock();
					m_CH4_Noise.LengthClock();
					break;
				case 3:
					break;
				case 4:
					m_CH1_Square.LengthClock();
					m_CH2_Square.LengthClock();
					m_CH3_Wave.LengthClock();
					m_CH4_Noise.LengthClock();
					break;
				case 5:
					break;
				case 6:
					m_CH1_Square.SweepClock();
					m_CH1_Square.LengthClock();
					m_CH2_Square.LengthClock();
					m_CH3_Wave.LengthClock();
					m_CH4_Noise.LengthClock();
					break;
				case 7: 
					m_CH1_Square.EnvelopeClock();
					m_CH2_Square.EnvelopeClock();
					m_CH4_Noise.EnvelopeClock();
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
		m_CH3_Wave.Clock();
		m_CH4_Noise.Clock();

		// only read the sample every (cpu frequency / 44100)
		if (m_CycleCount >= 95)
		{
			float tempVolume = 0.01f;

			// check to see if audio channels are enabled
			bool isCh1On = m_MMU.ReadRegisterBit(HW_NR52_SOUND_TOGGLE, NR52_CH1_ON);
			bool isCh2On = m_MMU.ReadRegisterBit(HW_NR52_SOUND_TOGGLE, NR52_CH2_ON);
			bool isCh3On = m_MMU.ReadRegisterBit(HW_NR52_SOUND_TOGGLE, NR52_CH3_ON);
			bool isCh4On = m_MMU.ReadRegisterBit(HW_NR52_SOUND_TOGGLE, NR52_CH4_ON);

			// get sample from each channel
			float ch1Sample = isCh1On ? static_cast<float>(m_CH1_Square.GetCurrentSample() * tempVolume) : 0;
			float ch2Sample = isCh2On ? static_cast<float>(m_CH2_Square.GetCurrentSample() * tempVolume) : 0;
			float ch3Sample = isCh3On ? static_cast<float>(m_CH3_Wave.GetCurrentSample()   * tempVolume) : 0;
			float ch4Sample = isCh4On ? static_cast<float>(m_CH4_Noise.GetCurrentSample()  *  tempVolume) : 0;

			// add samples to audio buffers
			// todo: we push twice.. once for left speaker, once for right speaker
			//	need to add stereo support
			m_CH1Buffer.push_back(ch1Sample);
			m_CH1Buffer.push_back(ch1Sample);

			m_CH2Buffer.push_back(ch2Sample);
			m_CH2Buffer.push_back(ch2Sample);

			m_CH3Buffer.push_back(ch3Sample);
			m_CH3Buffer.push_back(ch3Sample);

			m_CH4Buffer.push_back(ch4Sample);
			m_CH4Buffer.push_back(ch4Sample);

			m_MasterBuffer.push_back(ch1Sample + ch2Sample + ch3Sample + ch4Sample);
			m_MasterBuffer.push_back(ch1Sample + ch2Sample + ch3Sample + ch4Sample);

			//ringBuffer.Write(ch1Sample + ch2Sample); // left
			//ringBuffer.Write(ch1Sample + ch2Sample); // right
			
			m_CycleCount = 0;
		}

		if (m_MasterBuffer.size() >= 4096)
		{
			SDL_QueueAudio(m_SDLAudioDevice, m_MasterBuffer.data(), static_cast<Uint32>(m_MasterBuffer.size() * 4));

			m_CH1Buffer.clear();
			m_CH2Buffer.clear();
			m_CH3Buffer.clear();
			m_CH4Buffer.clear();
			m_MasterBuffer.clear();


			/*Uint32 queuedBytes = SDL_GetQueuedAudioSize(m_SDLAudioDevice);
			std::ostringstream stream;
			stream << "Audio Queued Bytes[End]: " << queuedBytes;
			Logger::Instance().Info(Core::Domain::APU, stream.str());*/

			//while (SDL_GetQueuedAudioSize(m_SDLAudioDevice) > 4096 * 4) {}
		}
	}

	void Apu::OnWrite(uint16_t address, uint8_t value)
	{
		if (address == HW_NR14_SOUND_CHANNEL_1_PERIOD_HIGH)
		{
			if (m_MMU.ReadRegisterBit(HW_NR14_SOUND_CHANNEL_1_PERIOD_HIGH, 0x80))
			{
				m_CH1_Square.Trigger();
			}
		}
		if (address == HW_NR24_SOUND_CHANNEL_2_PERIOD_HIGH)
		{
			if (m_MMU.ReadRegisterBit(HW_NR24_SOUND_CHANNEL_2_PERIOD_HIGH, 0x80))
			{
				m_CH2_Square.Trigger();
			}
		}
		if (address == HW_NR34_SOUND_CHANNEL_3_PERIOD_HIGH)
		{
			if (m_MMU.ReadRegisterBit(HW_NR34_SOUND_CHANNEL_3_PERIOD_HIGH, 0x80))
			{
				m_CH3_Wave.Trigger();
			}
		}
		if (address == HW_NR44_SOUND_CHANNEL_4_CONTROL)
		{
			if (m_MMU.ReadRegisterBit(HW_NR44_SOUND_CHANNEL_4_CONTROL, 0x80))
			{
				m_CH4_Noise.Trigger();
			}
		}
	}

	std::vector<float> Apu::GetMasterAudioBuffer()
	{
		return m_MasterBuffer;
	}

	std::vector<float> Apu::GetCh1AudioBuffer()
	{
		return m_CH1Buffer;
	}

	std::vector<float> Apu::GetCh2AudioBuffer()
	{
		return m_CH2Buffer;
	}

	std::vector<float> Apu::GetCh3AudioBuffer()
	{
		return m_CH3Buffer;
	}

	std::vector<float> Apu::GetCh4AudioBuffer()
	{
		return m_CH4Buffer;
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
			if (time >= 1.0 / freq) 
			{
				time -= 1.0 / freq;
			}
		}
	}

	void Apu::FeedAudioBuffer(uint8_t * stream, int len)
	{
		int samplesMissing = (len / 4) - ringBuffer.GetSize();
		if (samplesMissing > 0)
		{
			// just here so it doesn't get optimized away
			stream[0] = 0;
		}

		// The stream is just a pointer to an array of bytes. Since
		// we are using 32 bit floats for our audio samples, we need
		// to break the float into four distinct bytes
		for (int i = 0; i < len; i += 4)
		{
			float sample = ringBuffer.Read();
			
			// copy float to an array of 4 bytes
			uint8_t bytes[4];
			memcpy(bytes, &sample, sizeof(float));

			// set individual bytes
			stream[i] = bytes[0];
			stream[i + 1] = bytes[1];
			stream[i + 2] = bytes[2];
			stream[i + 3] = bytes[3];
		}
	}

	int Apu::GetRingBufferSize() const
	{
		return ringBuffer.GetSize();
	}
}