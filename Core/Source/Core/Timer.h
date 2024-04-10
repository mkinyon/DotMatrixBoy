
#pragma once
#include "Mmu.h"

namespace Core
{
	class Timer
	{
	public:
		Timer(Mmu& mmu);
		~Timer();

		void Clock();

	private:
		Mmu& m_MMU;
	};
}