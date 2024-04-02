
#pragma once

#include <map>
#include <string>
#include <functional>

namespace Core
{
	class InstructionSet
	{
	public:
		InstructionSet();
		~InstructionSet();

	public:
		typedef uint8_t(*OpcodeFunc)(class Cpu*);

		struct sInstructionDefinition
		{
			uint8_t length = 0;
			std::string nmemonic;
			OpcodeFunc instructionFunc;
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