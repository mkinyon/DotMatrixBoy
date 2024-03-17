#pragma once
#include <cstdint>
#include <vector>

namespace Core
{
	class RingBuffer
	{
	public:
		RingBuffer();
		~RingBuffer();

		void Reset();
		float Read();
		void Write(float data);
		int GetSize() const;

	private:
		uint32_t m_Size = 1024 * 1024;
		std::vector<float> m_Buffer;
		uint32_t m_ReadIndex = 0;
		uint32_t m_WriteIndex = 0;
	};
}