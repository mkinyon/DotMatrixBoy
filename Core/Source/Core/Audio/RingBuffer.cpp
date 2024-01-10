#include "RingBuffer.h"

namespace Core
{
	RingBuffer::RingBuffer() : buffer(size)
	{
		Reset();
	}

	RingBuffer::~RingBuffer() {}

	void RingBuffer::Reset()
	{
		readIndex = 0;
		writeIndex = 0;
	}

	float RingBuffer::Read()
	{
		// check if buffer is empty
		if (readIndex == writeIndex)
		{
			return 0.0f;
		}

		float data = buffer[readIndex];
		readIndex = (readIndex + 1) % size;
		return data;
	}

	void RingBuffer::Write(float data)
	{
		buffer[writeIndex] = data;
		writeIndex = (writeIndex + 1) % size;

		// handle buffer overflow
		if (writeIndex == readIndex)
		{
			readIndex = (readIndex + 1) % size;
		}
	}
}