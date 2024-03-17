#include "RingBuffer.h"

namespace Core
{
	RingBuffer::RingBuffer() : m_Buffer(m_Size)
	{
		Reset();
	}

	RingBuffer::~RingBuffer() {}

	void RingBuffer::Reset()
	{
		m_ReadIndex = 0;
		m_WriteIndex = 0;
	}

	float RingBuffer::Read()
	{
		// check if buffer is empty
		if (m_ReadIndex == m_WriteIndex)
		{
			return 0.0f;
		}

		float data = m_Buffer[m_ReadIndex];
		m_ReadIndex = (m_ReadIndex + 1) % m_Size;

		m_NumOfElements = GetSize();

		return data;
	}

	void RingBuffer::Write(float data)
	{
		m_Buffer[m_WriteIndex] = data;
		m_WriteIndex = (m_WriteIndex + 1) % m_Size;

		// handle buffer overflow
		if (m_WriteIndex == m_ReadIndex)
		{
			m_ReadIndex = (m_ReadIndex + 1) % m_Size;
		}

		m_NumOfElements = GetSize();
	}

	int RingBuffer::GetSize() const
	{
		// Calculate the difference between write and read indices
		int size = m_WriteIndex - m_ReadIndex;
		if (size < 0)
		{
			// Adjust for wrap-around in ring buffer
			size += m_Size;
		}
		
		return size;
	}
}