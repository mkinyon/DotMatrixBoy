
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
		struct sInstructionDefinition
		{
			uint8_t length = 0;
			std::string nmemonic;
			void* instructionFunc;
		};

		struct sInstruction
		{
			sInstructionDefinition definition;
			uint8_t opcode;
			uint8_t lowByte;
			uint8_t highByte;
		};

		std::map<uint8_t, sInstructionDefinition> m_InstructionMap;
		std::map<uint8_t, sInstructionDefinition> m_16BitInstructionMap;
	};
}