
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
			uint8_t opcode = 0;
			uint8_t lowByte = 0;
			uint8_t highByte = 0;
		};

		std::map<uint8_t, sInstructionDefinition> m_InstructionMap;
		std::map<uint8_t, sInstructionDefinition> m_16BitInstructionMap;
	};
}