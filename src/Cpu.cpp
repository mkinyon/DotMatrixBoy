#include "Cpu.h"

Cpu::Cpu() {}
Cpu::~Cpu() {}

void Cpu::clock( uint8_t opcode )
{
	//opcode = read(pc);

	switch (opcode)
	{
		case 0x00: // NOP
			break;
		case 0x01: // LD BC,d16
			break;
		default:
			// handle unknown opcode
			break;
	}

	return;
}

uint8_t Cpu::read(uint8_t address)
{
	return 0;//cart.read(address);
}


int Cpu::disassemble(std::vector<uint8_t> *rom, int pc)
{
	uint8_t opcode = rom->at(pc);
	int opBytes = 1;

	switch (opcode)
	{
		// Only advances the program counter by 1. Performs no other operations that would have an effect.
		// 
		// Opcode: 0x00
		// Number of Bytes : 1
		// Number of Cycles : 1
		// Flags : ----
		case 0x00: outputDisassembledInstruction("NOP", pc, rom, 1); break;

		// Load the 2 bytes of immediate data into register pair BC. The first byte of immediate data 
		// is the lower byte (i.e., bits 0-7), and the second byte of immediate data is the 
		// higher byte (i.e., bits 8-15).
		//
		// Opcode: 0x01
		// Number of Bytes : 3
		// Number of Cycles : 3
		// Flags : ----
		case 0x01: outputDisassembledInstruction("LD BC, d16", pc, rom, 3); opBytes = 3; break;

		// Store the contents of register A in the memory location specified by register pair BC.
		//
		// Opcode: 0x02
		// Number of Bytes : 1
		// Number of Cycles : 2
		// Flags : ----
		case 0x02: outputDisassembledInstruction("LD (BC)", pc, rom, 1); break;

		// Increment the contents of register pair BC by 1.
		//
		// Opcode: 0x03
		// Number of Bytes : 1
		// Number of Cycles : 2
		// Flags : ----
		case 0x03: outputDisassembledInstruction("INC BC", pc, rom, 1); break;

		// Increment the contents of register B by 1.
		//
		// Opcode: 0x04
		// Number of Bytes : 1
		// Number of Cycles : 1
		// Flags : Z 0 8 - bit -
		case 0x04: outputDisassembledInstruction("INC B", pc, rom, 1); break;

		// Decrement the contents of register B by 1.
		//
		// Opcode: 0x05
		// Number of Bytes : 1
		// Number of Cycles : 1
		// Flags : Z 1 8 - bit -
		case 0x05: outputDisassembledInstruction("DEC B", pc, rom, 1); break;

		// Load the 8-bit immediate operand d8 into register B.
		// 
		// Opcode: 0x06
		// Number of Bytes : 2
		// Number of Cycles : 2
		// Flags : ----
		case 0x06: outputDisassembledInstruction("LD B, d8", pc, rom, 2); opBytes = 2; break;

		// Rotate the contents of register A to the left. That is, the contents of bit 
		// 0 are copied to bit 1, and the previous contents of bit 1 (before the copy operation) 
		// are copied to bit 2. The same operation is repeated in sequence for the rest of the 
		// register. The contents of bit 7 are placed in both the CY flag and bit 0 of register A.
		//
		// Opcode: 0x07
		// Number of Bytes : 1
		// Number of Cycles : 1
		// Flags : 0 0 0 A7
		case 0x07: outputDisassembledInstruction("RCLA", pc, rom, 1); break;

		// Store the lower byte of stack pointer SP at the address specified by the 16-bit immediate 
		// operand a16, and store the upper byte of SP at address a16 + 1.
		//
		// Opcode: 0x08
		// Number of Bytes : 3
		// Number of Cycles : 5
		// Flags : ----
		case 0x08: outputDisassembledInstruction("LD (a16), SP", pc, rom, 3); opBytes = 3; break;

		// Add the contents of register pair BC to the contents of register pair HL, 
		// and store the results in register pair HL.
		//
		// Opcode: 0x09
		// Number of Bytes : 1
		// Number of Cycles : 2
		// Flags : -0 16 - bit 16 - bit
		case 0x09: outputDisassembledInstruction("ADD HL, BC", pc, rom, 1); break;

		// Load the 8-bit contents of memory specified by register pair BC into register A.
		//
		// Opcode: 0x0A
		// Number of Bytes : 1
		// Number of Cycles : 2
		// Flags : ----
		case 0x0A: outputDisassembledInstruction("LD A, (BC)", pc, rom, 1); break;

		// Decrement the contents of register pair BC by 1.
		//
		// Opcode: 0x0B
		// Number of Bytes : 1
		// Number of Cycles : 2
		// Flags : ----
		case 0x0B: outputDisassembledInstruction("DEC BC", pc, rom, 1); break;

		// Increment the contents of register C by 1.
		//
		// Opcode: 0x0C
		// Number of Bytes : 1
		// Number of Cycles : 1
		// Flags : Z 0 8 - bit -
		case 0x0C: outputDisassembledInstruction("INC C", pc, rom, 1); break;

		// Decrement the contents of register C by 1.
		//
		// Opcode: 0x0D
		// Number of Bytes : 1
		// Number of Cycles : 1
		// Flags : Z 1 8 - bit -
		case 0x0D: outputDisassembledInstruction("DEC C", pc, rom, 1); break;

		// Load the 8-bit immediate operand d8 into register C.
		//
		// Opcode: 0x0E
		// Number of Bytes : 2
		// Number of Cycles : 2
		// Flags : ----
		case 0x0E: outputDisassembledInstruction("LD C, d8", pc, rom, 2); opBytes = 2; break;

		// Rotate the contents of register A to the right. That is, the contents of bit 7 are copied to 
		// bit 6, and the previous contents of bit 6 (before the copy) are copied to bit 5. The same 
		// operation is repeated in sequence for the rest of the register. The contents of bit 0 are 
		// placed in both the CY flag and bit 7 of register A.
		//
		// Opcode: 0x0F
		// Number of Bytes : 1
		// Number of Cycles : 1
		// Flags : 0 0 0 A0
		case 0x0F: outputDisassembledInstruction("RRCA", pc, rom, 1); break;




		// Load the contents of register C into register B
		case 0x42: outputDisassembledInstruction("LD B, C", pc, rom, 1); break;

		// jump instructions
		case 0xC3: outputDisassembledInstruction("JMP NN NN", pc, rom, 3); opBytes = 3; break;
		case 0xE9: outputDisassembledInstruction("JMP", pc, rom, 1); break;

		default: outputDisassembledInstruction("", pc, rom, 1); break;
	}

	return opBytes;
}

void outputDisassembledInstruction(const char* instructionName, int pc, std::vector<uint8_t>* rom, int totalOpBytes)
{
	/* Ideal format
		0000 00       NOP
		0001 c3 d4 18 JMP    $18d4
	*/

	// print program counter address
	printf("%04x ", pc);
	printf(" ");

	// print address values
	if (totalOpBytes == 3)
	{
		printf("%02x ", (int)rom->at(pc));
		printf("%02x ", (int)rom->at(pc + 1));
		printf("%02x ", (int)rom->at(pc + 2));
	}
	else if (totalOpBytes == 2)
	{
		printf("%02x ", (int)rom->at(pc));
		printf("%02x ", (int)rom->at(pc + 1));
		printf("   ");
	}
	else
	{
		printf("%02x ", (int)rom->at(pc));
		printf("   ");
		printf("   ");
	}

	// print instruction name
	printf(" ");
	printf(instructionName);
	printf(" ");

	// print address
	if (totalOpBytes == 3)
	{
		printf("$");
		printf("%02x", (int)rom->at(pc + 2));
		printf("%02x", (int)rom->at(pc + 1));
	}
	else if (totalOpBytes == 2)
	{
		printf("$0x");
		printf("%02x", (int)rom->at(pc + 1));
	}

	// new line
	std::cout << "\n";
}