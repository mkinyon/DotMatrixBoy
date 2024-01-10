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

	private:
		uint32_t size = 1024 * 1024;
		std::vector<float> buffer;
		uint32_t readIndex = 0;
		uint32_t writeIndex = 0;
	};
}