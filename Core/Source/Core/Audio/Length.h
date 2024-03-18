#pragma once

#include <cstdint>

namespace Core
{
	class Length
	{
	public:
		void SetLength(uint16_t value, bool stopAfterLength);
		bool Clock();

	private:
		bool m_LengthStop = false;
		uint16_t m_Length = 0;
	};
}