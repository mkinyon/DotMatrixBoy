
#include "Envelope.h"

namespace Core
{
	void Envelope::SetEnvelope(uint8_t ticks, uint8_t startVolume, bool isIncreasing)
	{
		m_Ticks = ticks;
		m_Volume = startVolume;
		m_IsEnvIncreasing = isIncreasing;
	}

	void Envelope::Clock()
	{
		// if the ticks are zero, then there is no volume envelope to process
		if (m_Ticks == 0)
		{
			return;
		}

		m_ElapsedTicks++;
		m_ElapsedTicks = m_ElapsedTicks % m_Ticks;

		if (m_ElapsedTicks == 0)
		{
			if (!m_IsEnvIncreasing && m_Volume != 0)
			{
				m_Volume--;
			}
			else if (m_IsEnvIncreasing && m_Volume != 15)
			{
				m_Volume++;
			}
		}
	}

	uint8_t Envelope::GetVolume()
	{
		return m_Volume;
	}
}