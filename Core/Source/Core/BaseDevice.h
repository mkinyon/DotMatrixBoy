#pragma once
#include <cstdint>

namespace Core
{
	class BaseDevice
	{
	public:
		virtual void OnWrite(uint16_t address, uint8_t value) {};
	};
}