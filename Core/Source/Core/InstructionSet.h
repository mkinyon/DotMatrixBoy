
#pragma once
#include <map>
#include <string>

namespace Core
{
	class InstructionSet
	{
	public:
		InstructionSet();
		~InstructionSet();

	public:
		struct sCpuInstruction
		{
			uint8_t length = 0;
			std::string nmemonic;
		};

		std::map<uint8_t, sCpuInstruction> m_InstructionMap;
		std::map<uint8_t, sCpuInstruction> m_16BitInstructionMap;
	};
}