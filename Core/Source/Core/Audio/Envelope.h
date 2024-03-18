#pragma once

#include <cstdint>

namespace Core
{
	class Envelope
	{
	public:
		void SetEnvelope(uint8_t ticks, uint8_t startVolume, bool increasing);
		void Clock();
		uint8_t GetVolume();

	private:
		uint8_t m_Ticks = 0;
		uint8_t m_ElapsedTicks = 0;
		uint8_t m_Volume = 0;
		bool m_IsEnvIncreasing = false;
	};
}