#pragma once

#include "Mmu.h"

namespace Core
{
	class Apu
	{
	public:
		Apu(Mmu& mmu);
		~Apu();

		void Clock();
		void FeedAudioBuffer(uint8_t* stream, int len);

	private:
		Mmu& mmu;
	};
}