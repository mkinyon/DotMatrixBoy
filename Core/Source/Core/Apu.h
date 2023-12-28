#pragma once

namespace Core
{
	class Apu
	{
	public:
		Apu(GameBoy& gb);
		~Apu();

		void Clock();

	private:
		GameBoy& gameboy;
	};
}