
#include "Length.h"

namespace Core
{
	void Length::SetLength(uint16_t value, bool stopAfterLength)
	{
		m_Length = m_Length == 0 ? value : m_Length;
		m_LengthStop = stopAfterLength;
	}

	bool Length::Clock()
	{
		m_Length -= m_Length != 0 ? 1 : 0;
		return !m_LengthStop || (m_Length != 0 && m_LengthStop);
	}
}