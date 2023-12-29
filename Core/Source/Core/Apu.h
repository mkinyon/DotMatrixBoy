#pragma once

namespace Core
{
	class Apu
	{
	public:
		Apu(GameBoy& gb);
		~Apu();

		void Clock();
		void FeedAudioBuffer(uint8_t* stream, int len);

	private:
		GameBoy& gameboy;
	};
}