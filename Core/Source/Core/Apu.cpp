
#include "Apu.h"

#include <stdint.h>

namespace Core
{
	Apu::Apu(Mmu& mmu) : mmu(mmu) {}
	Apu::~Apu() {}

	void Apu::Clock()
	{

	}

	void Apu::FeedAudioBuffer(uint8_t * stream, int len)
	{
		// generating a square wave
		const int sampleRate = 44100;
		static double time = 0.0;
		const double freq = static_cast<double>(440);
		const double amplitude = 10;

		for (int i = 0; i < len; ++i)
		{
			// Toggle between two amplitude values
			double sample = (time < 0.5 / freq) ? amplitude : -amplitude;

			int16_t intSample = static_cast<int16_t>(sample * 32767.0);  // Convert to signed 16-bit

			//stream[i] = static_cast<uint8_t>(intSample & 0xFF);            // Low byte
			//stream[i + 1] = static_cast<uint8_t>((intSample >> 8) & 0xFF);  // High byte

			time += 1.0 / sampleRate;
			if (time >= 1.0 / freq) {
				time -= 1.0 / freq;
			}
		}
	}
}