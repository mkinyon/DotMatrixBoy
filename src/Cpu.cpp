#include "Cpu.h"

struct Cpu::cpuFlags
{
	// Flag register
	uint8_t z : 1; // 7th bit: Zero flag (Z)
	uint8_t n : 1; // 6th bit: Subtract flag (N)
	uint8_t h : 1; // 5th bit: Half Carry Flag (H)
	uint8_t c : 1; // 4th bit: Carry flag (C)
	uint8_t pad : 4; // 3rd to 0 bit: not used
};

struct Cpu::cpuState
{
	// 8 bit registers
	// Note: Some instructions can pair two registers as 16 bit registers.
	//  The pairings are AF, BC, DE, & HL.
	uint8_t a = 0x00;
	uint8_t b = 0x00;
	uint8_t c = 0x00;
	uint8_t d = 0x00;
	uint8_t e = 0x00;
	uint8_t f = 0x00;
	uint8_t h = 0x00;
	uint8_t l = 0x00;

	uint16_t sp = 0x00; // stack pointer
	uint16_t pc = 0x100; // the gameboy program counter starts at $100

	std::vector<uint8_t>* memory;
	struct cpuFlags flags;

	uint8_t	int_enable;
};

Cpu::cpuState state;

Cpu::Cpu() {}
Cpu::~Cpu() {}

void Cpu::loadCartData(std::vector<uint8_t>* rom)
{
	state.memory = rom;
}

void Cpu::clock()
{
	uint8_t* opcode = &state.memory->at(state.pc);

	disassemble(state.memory, state.pc);

	state.pc += 1;  //for the opcode 

	switch (*opcode)
	{
		// Only advances the program counter by 1. Performs no other operations that would have an effect.
		case 0x00:
			break;

		// Load the 2 bytes of immediate data into register pair BC. The first byte of immediate data 
		// is the lower byte (i.e., bits 0-7), and the second byte of immediate data is the 
		// higher byte (i.e., bits 8-15).
		case 0x01:
		{
			state.c = opcode[1];
			state.b = opcode[2];
			state.pc += 2;
			break;
		}

		// Store the contents of register A in the memory location specified by register pair BC.
		case 0x02:
		{
			uint16_t offset = (state.b << 8) | state.c;
			WriteToMemory(offset, state.a);
			break;
		}

		// Increment the contents of register pair BC by 1.
		case 0x03:
		{
			state.c = state.c + 1;
			if (state.c == 0)
				state.b = state.b + 1;
			break;
		}

		// Load the 8 - bit immediate operand d8 into register B.
		case 0x06:
		{
			state.b = opcode[1];
			state.pc++;
			break;
		}

		// Load the 8-bit immediate operand d8 into register C.
		case 0x0E:
		{
			state.c = opcode[1];
			state.pc++;
			break;
		}


		// If the Z flag is 0, jump s8 steps from the current address stored in the 
		// program counter (PC). If not, the instruction following the current 
		// JP instruction is executed (as usual).
		case 0x20:
		{
			// note: "s8" in the description refers to a signed char
			int8_t offset = opcode[1];
			if (state.flags.z == 0)
			{
				state.pc += offset;
			}
			else
			{
				state.pc++;
			}
		}

		// Load the 2 bytes of immediate data into register pair HL.

		// The first byte of immediate data is the lower byte (i.e., bits 0 - 7), 
		// and the second byte of immediate data is the higher byte(i.e., bits 8 - 15).
		case 0x21:
		{
			state.h = opcode[1];
			state.l = opcode[2];
			state.pc += 2;
			break;
		}

		// Load the contents of register A into register A.
		case 0x7F:
		{
			state.a = state.a;
			break;
		}

		// Take the logical exclusive-OR for each bit of the contents of register A and 
		// the contents of register A, and store the results in register A.
		case 0xAF:
		{
			state.a = state.a ^ state.a;
			state.flags.z = 1;
			state.flags.c = 0;
			state.flags.h = 0;
			state.flags.n = 0;
			break;
		}


		// Store the contents of register A into the memory location specified by
		//  register pair HL, and simultaneously decrement the contents of HL.
		case 0x32:
		{
			uint16_t offset = (opcode[2] << 8) | (opcode[1]);
			WriteToMemory(offset, state.a);
			state.pc++;
			break;
		}

		// Load the 8-bit immediate operand d8 into register A.
		case 0x3E:
		{
			state.a = opcode[1];
			state.pc++;
			break;
		}

		// Load the 16-bit immediate operand a16 into the program counter (PC). a16 specifies 
		// the address of the subsequently executed instruction.

		// The second byte of the object code(immediately following the opcode) corresponds 
		// to the lower - order byte of a16(bits 0 - 7), and the third byte of the object 
		// code corresponds to the higher - order byte(bits 8 - 15).
		case 0xC3:
		{
			uint16_t offset = (opcode[2] << 8) | (opcode[1]);
			state.pc = offset;
			break;
		}

		// Store the contents of register A in the internal RAM, port register, or mode register 
		// at the address in the range 0xFF00-0xFFFF specified by the 8-bit immediate operand a8.

		// Note: Should specify a 16 - bit address in the mnemonic portion for a8, although 
		// the immediate operand only has the lower - order 8 bits.

		// 0xFF00 - 0xFF7F : Port / Mode registers, control register, sound register
		// 0xFF80 - 0xFFFE : Working & Stack RAM(127 bytes)
		// 0xFFFF : Interrupt Enable Register
		case 0xE0:
		{
			unimplementedInstruction(state, *opcode);
			break;
		}

		// Reset the interrupt master enable (IME) flag and prohibit maskable interrupts.

		// Even if a DI instruction is executed in an interrupt routine, the IME 
		// flag is set if a return is performed with a RETI instruction.
		case 0xF3:
		{
			state.int_enable = 0;
			break;
		}

		// Load the contents of register pair HL into the stack pointer SP.
		case 0xF9:
		{
			state.sp = (state.l << 8) | (state.h);
			break;
		}

		// Push the current value of the program counter PC onto the memory stack, and load into PC 
		// the 8th byte of page 0 memory addresses, 0x38. The next instruction is fetched from the 
		// address specified by the new content of PC(as usual).

		// With the push, the contents of the stack pointer SP are decremented by 1, and the 
		// higher - order byte of PC is loaded in the memory address specified by the new SP 
		// value. The value of SP is then again decremented by 1, and the lower - order byte
		// of the PC is loaded in the memory address specified by that value of SP.

		// The RST instruction can be used to jump to 1 of 8 addresses. Because all of the 
		// addresses are held in page 0 memory, 0x00 is loaded in the higher - order byte 
		// of the PC, and 0x38 is loaded in the lower - order byte.
		case 0xFF:
		{
			uint16_t ret = state.pc + 2;
			WriteToMemory(state.sp - 1, (ret >> 8) & 0xff);
			WriteToMemory(state.sp - 2, (ret & 0xff));
			state.sp = state.sp - 2;
			state.pc = 0x38;
			break;
		}

		default: 
			unimplementedInstruction(state, *opcode); 
			break;
	}
}

void WriteToMemory(uint16_t address, uint8_t value)
{
	if (address < 0x2000)
	{
		return;
	}
	if (address >= 0x4000)
	{
		return;
	}

	state.memory->at(address) = value;
}

void unimplementedInstruction(Cpu::cpuState &state, uint8_t opcode)
{
	//pc will have advanced one, so undo that 
	printf("\n");
	printf("Error: Unimplemented instruction: %02x \n", opcode);
	printf("######################################################\n");
	printf("# CPU Details:\n");
	printf("# Program Counter: %04x \n", state.pc);
	printf("# Registers: \n");
	printf("#    A: %02x \n", state.a);
	printf("#    B: %02x \n", state.b);
	printf("#    C: %02x \n", state.c);
	printf("#    D: %02x \n", state.d);
	printf("#    E: %02x \n", state.e);
	printf("#    F: %02x \n", state.f);
	printf("#    H: %02x \n", state.h);
	printf("#    L: %02x \n", state.l);
	printf("# Flags:\n");
	printf("#    Zero flag (Z): %02x \n", state.flags.z );
	printf("#    Subtract flag (N): %02x \n", state.flags.z);
	printf("#    Half Carry Flag (H): %02x \n", state.flags.z);
	printf("#    Carry flag (C): %02x \n", state.flags.z);
	printf("######################################################\n");
	exit(1);
}

int Cpu::disassemble(std::vector<uint8_t> *rom, int pc)
{
	uint8_t opcode = rom->at(pc);
	int opBytes = 1;

	switch (opcode)
	{
		case 0x00: outputDisassembledInstruction("NOP", pc, rom, 1); break;
		case 0x01: outputDisassembledInstruction("LD BC, d16", pc, rom, 3); opBytes = 3; break;
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

		// Execution of a STOP instruction stops both the system clock and oscillator circuit. STOP mode is 
		// entered and the LCD controller also stops. However, the status of the internal RAM register ports remains unchanged.
		// 
		// STOP mode can be cancelled by a reset signal.
		// 
		// If the RESET terminal goes LOW in STOP mode, it becomes that of a normal reset status.
		// 
		// The following conditions should be met before a STOP instruction is executed and stop mode is entered:
		//  - All interrupt - enable(IE) flags are reset.
		//  - Input to P10 - P13 is LOW for all.
		// 
		// Opcode: 0x1000
		// Number of Bytes : 2
		// Number of Cycles : 1
		// Flags : ----
		case 0x10: outputDisassembledInstruction("STOP", pc, rom, 2); opBytes = 2; break;
		case 0x11: outputDisassembledInstruction("LD DE, d16", pc, rom, 3); opBytes = 3; break;
		case 0x12: outputDisassembledInstruction("LD (DE), A", pc, rom, 1); break;
		case 0x13: outputDisassembledInstruction("INC DE", pc, rom, 1); break;
		case 0x14: outputDisassembledInstruction("INC D", pc, rom, 1); break;
		case 0x15: outputDisassembledInstruction("DEC D", pc, rom, 1); break;
		case 0x16: outputDisassembledInstruction("LD D, d8", pc, rom, 2); opBytes = 2; break;
		case 0x17: outputDisassembledInstruction("RLA", pc, rom, 1); break;
		case 0x18: outputDisassembledInstruction("JR s8", pc, rom, 2); opBytes = 2; break;
		case 0x19: outputDisassembledInstruction("ADD HL, DE", pc, rom, 1); break;
		case 0x1A: outputDisassembledInstruction("LD A, (DE)", pc, rom, 1); break;
		case 0x1B: outputDisassembledInstruction("DEC DE", pc, rom, 1); break;
		case 0x1C: outputDisassembledInstruction("INC E", pc, rom, 1); break;
		case 0x1D: outputDisassembledInstruction("DEC E", pc, rom, 1); break;
		case 0x1E: outputDisassembledInstruction("LD E, d8", pc, rom, 2); opBytes = 2; break;
		case 0x1F: outputDisassembledInstruction("RRA", pc, rom, 1); break;

		case 0x20: outputDisassembledInstruction("JR NZ, s8", pc, rom, 2); opBytes = 2; break;
		case 0x21: outputDisassembledInstruction("LD HL, d16", pc, rom, 3); opBytes = 3; break;
		case 0x22: outputDisassembledInstruction("LD (HL+), A", pc, rom, 1); break;
		case 0x23: outputDisassembledInstruction("INC HL", pc, rom, 1); break;
		case 0x24: outputDisassembledInstruction("INC H", pc, rom, 1); break;
		case 0x25: outputDisassembledInstruction("DEC H", pc, rom, 1); break;
		case 0x26: outputDisassembledInstruction("LD H, d8", pc, rom, 2); opBytes = 2; break;
		case 0x27: outputDisassembledInstruction("DAA", pc, rom, 1); break;
		case 0x28: outputDisassembledInstruction("JR Z, s8", pc, rom, 2); opBytes = 2; break;
		case 0x29: outputDisassembledInstruction("ADD HL, HL", pc, rom, 1); break;
		case 0x2A: outputDisassembledInstruction("LD A, (HL+)", pc, rom, 1); break;
		case 0x2B: outputDisassembledInstruction("DEC HL", pc, rom, 1); break;
		case 0x2C: outputDisassembledInstruction("INC L", pc, rom, 1); break;
		case 0x2D: outputDisassembledInstruction("DEC L", pc, rom, 1); break;
		case 0x2E: outputDisassembledInstruction("LD L, d8", pc, rom, 2); opBytes = 2; break;
		case 0x2F: outputDisassembledInstruction("CPL", pc, rom, 1); break;

		case 0x30: outputDisassembledInstruction("JR NC, s8", pc, rom, 2); opBytes = 2; break;
		case 0x31: outputDisassembledInstruction("LD SP, d16", pc, rom, 3); opBytes = 3; break;
		case 0x32: outputDisassembledInstruction("LD (HL-), A", pc, rom, 1); break;
		case 0x33: outputDisassembledInstruction("INC SP", pc, rom, 1); break;
		case 0x34: outputDisassembledInstruction("INC (HL)", pc, rom, 1); break;
		case 0x35: outputDisassembledInstruction("DEC (HL)", pc, rom, 1); break;
		case 0x36: outputDisassembledInstruction("LD (HL), d8", pc, rom, 2); opBytes = 2; break;
		case 0x37: outputDisassembledInstruction("SCF", pc, rom, 1); break;
		case 0x38: outputDisassembledInstruction("JR C, s8", pc, rom, 2); opBytes = 2; break;
		case 0x39: outputDisassembledInstruction("ADD HL, SP", pc, rom, 1); break;
		case 0x3A: outputDisassembledInstruction("LD A, (HL-)", pc, rom, 1); break;
		case 0x3B: outputDisassembledInstruction("DEC SP", pc, rom, 1); break;
		case 0x3C: outputDisassembledInstruction("INC A", pc, rom, 1); break;
		case 0x3D: outputDisassembledInstruction("DEC A", pc, rom, 1); break;
		case 0x3E: outputDisassembledInstruction("LD A, d8", pc, rom, 2); opBytes = 2; break;
		case 0x3F: outputDisassembledInstruction("CCF", pc, rom, 1); break;
		
		case 0x40: outputDisassembledInstruction("LD B, B", pc, rom, 1); break;
		case 0x41: outputDisassembledInstruction("LD B, C", pc, rom, 1); break;
		case 0x42: outputDisassembledInstruction("LD B, D", pc, rom, 1); break;
		case 0x43: outputDisassembledInstruction("LD B, E", pc, rom, 1); break;
		case 0x44: outputDisassembledInstruction("LD B, H", pc, rom, 1); break;
		case 0x45: outputDisassembledInstruction("LD B, L", pc, rom, 1); break;
		case 0x46: outputDisassembledInstruction("LD B, (HL)", pc, rom, 1); break;
		case 0x47: outputDisassembledInstruction("LD B, A", pc, rom, 1); break;
		case 0x48: outputDisassembledInstruction("LD C, B", pc, rom, 1); break;
		case 0x49: outputDisassembledInstruction("LD C, C", pc, rom, 1); break;
		case 0x4A: outputDisassembledInstruction("LD C, D", pc, rom, 1); break;
		case 0x4B: outputDisassembledInstruction("LD C, E", pc, rom, 1); break;
		case 0x4C: outputDisassembledInstruction("LD C, H", pc, rom, 1); break;
		case 0x4D: outputDisassembledInstruction("LD C, L", pc, rom, 1); break;
		case 0x4E: outputDisassembledInstruction("LD C, (HL)", pc, rom, 1); break;
		case 0x4F: outputDisassembledInstruction("LD C, A", pc, rom, 1); break;
		
		case 0x50: outputDisassembledInstruction("LD D, B", pc, rom, 1); break;
		case 0x51: outputDisassembledInstruction("LD D, C", pc, rom, 1); break;
		case 0x52: outputDisassembledInstruction("LD D, D", pc, rom, 1); break;
		case 0x53: outputDisassembledInstruction("LD D, E", pc, rom, 1); break;
		case 0x54: outputDisassembledInstruction("LD D, H", pc, rom, 1); break;
		case 0x55: outputDisassembledInstruction("LD D, L", pc, rom, 1); break;
		case 0x56: outputDisassembledInstruction("LD D, (HL)", pc, rom, 1); break;
		case 0x57: outputDisassembledInstruction("LD D, A", pc, rom, 1); break;
		case 0x58: outputDisassembledInstruction("LD E, B", pc, rom, 1); break;
		case 0x59: outputDisassembledInstruction("LD E, C", pc, rom, 1); break;
		case 0x5A: outputDisassembledInstruction("LD E, D", pc, rom, 1); break;
		case 0x5B: outputDisassembledInstruction("LD E, E", pc, rom, 1); break;
		case 0x5C: outputDisassembledInstruction("LD E, H", pc, rom, 1); break;
		case 0x5D: outputDisassembledInstruction("LD E, L", pc, rom, 1); break;
		case 0x5E: outputDisassembledInstruction("LD E, (HL)", pc, rom, 1); break;
		case 0x5F: outputDisassembledInstruction("LD E, A", pc, rom, 1); break;
		
		case 0x60: outputDisassembledInstruction("LD H, B", pc, rom, 1); break;
		case 0x61: outputDisassembledInstruction("LD H, C", pc, rom, 1); break;
		case 0x62: outputDisassembledInstruction("LD H, D", pc, rom, 1); break;
		case 0x63: outputDisassembledInstruction("LD H, E", pc, rom, 1); break;
		case 0x64: outputDisassembledInstruction("LD H, H", pc, rom, 1); break;
		case 0x65: outputDisassembledInstruction("LD H, L", pc, rom, 1); break;
		case 0x66: outputDisassembledInstruction("LD H, (HL)", pc, rom, 1); break;
		case 0x67: outputDisassembledInstruction("LD H, A", pc, rom, 1); break;
		case 0x68: outputDisassembledInstruction("LD L, B", pc, rom, 1); break;
		case 0x69: outputDisassembledInstruction("LD L, C", pc, rom, 1); break;
		case 0x6A: outputDisassembledInstruction("LD L, D", pc, rom, 1); break;
		case 0x6B: outputDisassembledInstruction("LD L, E", pc, rom, 1); break;
		case 0x6C: outputDisassembledInstruction("LD L, H", pc, rom, 1); break;
		case 0x6D: outputDisassembledInstruction("LD L, L", pc, rom, 1); break;
		case 0x6E: outputDisassembledInstruction("LD L, (HL)", pc, rom, 1); break;
		case 0x6F: outputDisassembledInstruction("LD L, A", pc, rom, 1); break;
		
		case 0x70: outputDisassembledInstruction("LD (HL), B", pc, rom, 1); break;
		case 0x71: outputDisassembledInstruction("LD (HL), C", pc, rom, 1); break;
		case 0x72: outputDisassembledInstruction("LD (HL), D", pc, rom, 1); break;
		case 0x73: outputDisassembledInstruction("LD (HL), E", pc, rom, 1); break;
		case 0x74: outputDisassembledInstruction("LD (HL), H", pc, rom, 1); break;
		case 0x75: outputDisassembledInstruction("LD (HL), L", pc, rom, 1); break;
		case 0x76: outputDisassembledInstruction("HALT", pc, rom, 1); break;
		case 0x77: outputDisassembledInstruction("LD (HL), A", pc, rom, 1); break;
		case 0x78: outputDisassembledInstruction("LD A, B", pc, rom, 1); break;
		case 0x79: outputDisassembledInstruction("LD A, C", pc, rom, 1); break;
		case 0x7A: outputDisassembledInstruction("LD A, D", pc, rom, 1); break;
		case 0x7B: outputDisassembledInstruction("LD A, E", pc, rom, 1); break;
		case 0x7C: outputDisassembledInstruction("LD A, H", pc, rom, 1); break;
		case 0x7D: outputDisassembledInstruction("LD A, L", pc, rom, 1); break;
		case 0x7E: outputDisassembledInstruction("LD A, (HL)", pc, rom, 1); break;
		case 0x7F: outputDisassembledInstruction("LD A, A", pc, rom, 1); break;
		
		case 0x80: outputDisassembledInstruction("ADD A, B", pc, rom, 1); break;
		case 0x81: outputDisassembledInstruction("ADD A, C", pc, rom, 1); break;
		case 0x82: outputDisassembledInstruction("ADD A, D", pc, rom, 1); break;
		case 0x83: outputDisassembledInstruction("ADD A, E", pc, rom, 1); break;
		case 0x84: outputDisassembledInstruction("ADD A, H", pc, rom, 1); break;
		case 0x85: outputDisassembledInstruction("ADD A, L", pc, rom, 1); break;
		case 0x86: outputDisassembledInstruction("ADD A, (HL)", pc, rom, 1); break;
		case 0x87: outputDisassembledInstruction("ADD A, A", pc, rom, 1); break;
		case 0x88: outputDisassembledInstruction("ADC A, B", pc, rom, 1); break;
		case 0x89: outputDisassembledInstruction("ADC A, C", pc, rom, 1); break;
		case 0x8A: outputDisassembledInstruction("ADC A, D", pc, rom, 1); break;
		case 0x8B: outputDisassembledInstruction("ADC A, E", pc, rom, 1); break;
		case 0x8C: outputDisassembledInstruction("ADC A, H", pc, rom, 1); break;
		case 0x8D: outputDisassembledInstruction("ADC A, L", pc, rom, 1); break;
		case 0x8E: outputDisassembledInstruction("ADC A, (HL)", pc, rom, 1); break;
		case 0x8F: outputDisassembledInstruction("ADC A, A", pc, rom, 1); break;
		
		case 0x90: outputDisassembledInstruction("SUB B", pc, rom, 1); break;
		case 0x91: outputDisassembledInstruction("SUB C", pc, rom, 1); break;
		case 0x92: outputDisassembledInstruction("SUB D", pc, rom, 1); break;
		case 0x93: outputDisassembledInstruction("SUB E", pc, rom, 1); break;
		case 0x94: outputDisassembledInstruction("SUB H", pc, rom, 1); break;
		case 0x95: outputDisassembledInstruction("SUB L", pc, rom, 1); break;
		case 0x96: outputDisassembledInstruction("SUB (HL)", pc, rom, 1); break;
		case 0x97: outputDisassembledInstruction("SUB A", pc, rom, 1); break;
		case 0x98: outputDisassembledInstruction("SBC A, B", pc, rom, 1); break;
		case 0x99: outputDisassembledInstruction("SBC A, C", pc, rom, 1); break;
		case 0x9A: outputDisassembledInstruction("SBC A, D", pc, rom, 1); break;
		case 0x9B: outputDisassembledInstruction("SBC A, E", pc, rom, 1); break;
		case 0x9C: outputDisassembledInstruction("SBC A, H", pc, rom, 1); break;
		case 0x9D: outputDisassembledInstruction("SBC A, L", pc, rom, 1); break;
		case 0x9E: outputDisassembledInstruction("SBC A, (HL)", pc, rom, 1); break;
		case 0x9F: outputDisassembledInstruction("SBC A, A", pc, rom, 1); break;
		
		case 0xA0: outputDisassembledInstruction("AND B", pc, rom, 1); break;
		case 0xA1: outputDisassembledInstruction("AND C", pc, rom, 1); break;
		case 0xA2: outputDisassembledInstruction("AND D", pc, rom, 1); break;
		case 0xA3: outputDisassembledInstruction("AND E", pc, rom, 1); break;
		case 0xA4: outputDisassembledInstruction("AND H", pc, rom, 1); break;
		case 0xA5: outputDisassembledInstruction("AND L", pc, rom, 1); break;
		case 0xA6: outputDisassembledInstruction("AND (HL)", pc, rom, 1); break;
		case 0xA7: outputDisassembledInstruction("AND A", pc, rom, 1); break;
		case 0xA8: outputDisassembledInstruction("XOR B", pc, rom, 1); break;
		case 0xA9: outputDisassembledInstruction("XOR C", pc, rom, 1); break;
		case 0xAA: outputDisassembledInstruction("XOR D", pc, rom, 1); break;
		case 0xAB: outputDisassembledInstruction("XOR E", pc, rom, 1); break;
		case 0xAC: outputDisassembledInstruction("XOR H", pc, rom, 1); break;
		case 0xAD: outputDisassembledInstruction("XOR L", pc, rom, 1); break;
		case 0xAE: outputDisassembledInstruction("XOR (HL)", pc, rom, 1); break;
		case 0xAF: outputDisassembledInstruction("XOR A", pc, rom, 1); break;
		
		case 0xB0: outputDisassembledInstruction("OR B", pc, rom, 1); break;
		case 0xB1: outputDisassembledInstruction("OR C", pc, rom, 1); break;
		case 0xB2: outputDisassembledInstruction("OR D", pc, rom, 1); break;
		case 0xB3: outputDisassembledInstruction("OR E", pc, rom, 1); break;
		case 0xB4: outputDisassembledInstruction("OR H", pc, rom, 1); break;
		case 0xB5: outputDisassembledInstruction("OR L", pc, rom, 1); break;
		case 0xB6: outputDisassembledInstruction("OR (HL)", pc, rom, 1); break;
		case 0xB7: outputDisassembledInstruction("OR A", pc, rom, 1); break;
		case 0xB8: outputDisassembledInstruction("CP B", pc, rom, 1); break;
		case 0xB9: outputDisassembledInstruction("CP C", pc, rom, 1); break;
		case 0xBA: outputDisassembledInstruction("CP D", pc, rom, 1); break;
		case 0xBB: outputDisassembledInstruction("CP E", pc, rom, 1); break;
		case 0xBC: outputDisassembledInstruction("CP H", pc, rom, 1); break;
		case 0xBD: outputDisassembledInstruction("CP L", pc, rom, 1); break;
		case 0xBE: outputDisassembledInstruction("CP (HL)", pc, rom, 1); break;
		case 0xBF: outputDisassembledInstruction("CP A", pc, rom, 1); break;
		
		case 0xC0: outputDisassembledInstruction("RET NZ", pc, rom, 1); break;
		case 0xC1: outputDisassembledInstruction("POP BC", pc, rom, 1); break;
		case 0xC2: outputDisassembledInstruction("JP NZ, a16", pc, rom, 3); opBytes = 3; break;
		case 0xC3: outputDisassembledInstruction("JP a16", pc, rom, 3); opBytes = 3; break;
		case 0xC4: outputDisassembledInstruction("CALL NZ, a16", pc, rom, 3); opBytes = 3; break;
		case 0xC5: outputDisassembledInstruction("PUSH BC", pc, rom, 1); break;
		case 0xC6: outputDisassembledInstruction("ADD A, d8", pc, rom, 2); opBytes = 2; break;
		case 0xC7: outputDisassembledInstruction("RST 0", pc, rom, 1); break;
		case 0xC8: outputDisassembledInstruction("RET Z", pc, rom, 1); break;
		case 0xC9: outputDisassembledInstruction("RET", pc, rom, 1); break;

		// Address CB triggers a 16 bit opcode 
		case 0xCB: 
		{
			uint8_t firstChar = rom->at(pc);
			uint8_t secondChar = rom->at(pc + 1);
			uint16_t opcode16 = ((uint16_t)firstChar << 8) | secondChar;
			disasseble16bit(rom, opcode16, pc);
			opBytes = 2;
			break;
		}

		case 0xCA: outputDisassembledInstruction("JP Z, a16", pc, rom, 3); opBytes = 3; break;
		case 0xCC: outputDisassembledInstruction("CALL Z, a16", pc, rom, 3); opBytes = 3; break;
		case 0xCD: outputDisassembledInstruction("CALL a16", pc, rom, 3); opBytes = 3; break;
		case 0xCE: outputDisassembledInstruction("ADC A, d8", pc, rom, 2); opBytes = 2; break;
		case 0xCF: outputDisassembledInstruction("RST 1", pc, rom, 1); break;
		
		case 0xD0: outputDisassembledInstruction("RET NC", pc, rom, 1); break;
		case 0xD1: outputDisassembledInstruction("POP DE", pc, rom, 1); break;
		case 0xD2: outputDisassembledInstruction("JP NC, a16", pc, rom, 3); opBytes = 3; break;
		case 0xD4: outputDisassembledInstruction("CALL NC, a16", pc, rom, 3); opBytes = 3; break;
		case 0xD5: outputDisassembledInstruction("PUSH DE", pc, rom, 1); break;
		case 0xD6: outputDisassembledInstruction("SUB d8", pc, rom, 2); opBytes = 2; break;
		case 0xD7: outputDisassembledInstruction("RST 2", pc, rom, 1); break;
		case 0xD8: outputDisassembledInstruction("RET C", pc, rom, 1); break;
		case 0xD9: outputDisassembledInstruction("RETI", pc, rom, 1); break;
		case 0xDA: outputDisassembledInstruction("JP C, a16", pc, rom, 3); opBytes = 3; break;
		case 0xDC: outputDisassembledInstruction("CALL C, a16", pc, rom, 3); opBytes = 3; break;
		case 0xDE: outputDisassembledInstruction("SBC A, d8", pc, rom, 2); opBytes = 2; break;
		case 0xDF: outputDisassembledInstruction("RST 3", pc, rom, 1); break;
		
		case 0xE0: outputDisassembledInstruction("LD (a8), A", pc, rom, 2); opBytes = 2; break;
		case 0xE1: outputDisassembledInstruction("POP HL", pc, rom, 1); break;
		case 0xE2: outputDisassembledInstruction("LD (C), A", pc, rom, 1); break;
		case 0xE5: outputDisassembledInstruction("PUSH HL", pc, rom, 1); break;
		case 0xE6: outputDisassembledInstruction("AND d8", pc, rom, 2); opBytes = 2; break;
		case 0xE7: outputDisassembledInstruction("RST 4", pc, rom, 1); break;
		case 0xE8: outputDisassembledInstruction("ADD SP, s8", pc, rom, 2); opBytes = 2; break;
		case 0xE9: outputDisassembledInstruction("JP HL", pc, rom, 1); break;
		case 0xEA: outputDisassembledInstruction("LD (a16), A", pc, rom, 3); opBytes = 3; break;
		case 0xEE: outputDisassembledInstruction("XOR d8", pc, rom, 2); opBytes = 2; break;
		case 0xEF: outputDisassembledInstruction("RST 5", pc, rom, 1); break;
		
		case 0xF0: outputDisassembledInstruction("LD A, (a8)", pc, rom, 2); opBytes = 2; break;
		case 0xF1: outputDisassembledInstruction("POP AF", pc, rom, 1); break;
		case 0xF2: outputDisassembledInstruction("LD A, (C)", pc, rom, 1); break;
		case 0xF3: outputDisassembledInstruction("DI", pc, rom, 1); break;
		case 0xF5: outputDisassembledInstruction("PUSH AF", pc, rom, 1); break;
		case 0xF6: outputDisassembledInstruction("OR d8", pc, rom, 2); opBytes = 2; break;
		case 0xF7: outputDisassembledInstruction("RST 6", pc, rom, 1); break;
		case 0xF8: outputDisassembledInstruction("LD HL, SP+s8", pc, rom, 2); opBytes = 2; break;
		case 0xF9: outputDisassembledInstruction("LD SP, HL", pc, rom, 1); break;
		case 0xFA: outputDisassembledInstruction("LD A, (a16)", pc, rom, 3); opBytes = 3; break;
		case 0xFB: outputDisassembledInstruction("EI", pc, rom, 1); break;
		case 0xFE: outputDisassembledInstruction("CP d8", pc, rom, 2); opBytes = 2; break;
		case 0xFF: outputDisassembledInstruction("RST 7", pc, rom, 1); break;

		default: outputDisassembledInstruction("", pc, rom, 1); break;
	}

	return opBytes;
}

void disasseble16bit(std::vector<uint8_t>* rom, uint16_t opcode, int pc)
{
	// Note: All 16 bit opcodes appear to be 2 bytes.

	switch (opcode)
	{
		case 0xCB00: outputDisassembledInstruction("RLC B", pc, rom, 2); break;
		case 0xCB01: outputDisassembledInstruction("RLC C", pc, rom, 2); break;
		case 0xCB02: outputDisassembledInstruction("RLC D", pc, rom, 2); break;
		case 0xCB03: outputDisassembledInstruction("RLC E", pc, rom, 2); break;
		case 0xCB04: outputDisassembledInstruction("RLC H", pc, rom, 2); break;
		case 0xCB05: outputDisassembledInstruction("RLC L", pc, rom, 2); break;
		case 0xCB06: outputDisassembledInstruction("RLC (HL)", pc, rom, 2); break;
		case 0xCB07: outputDisassembledInstruction("RLC A", pc, rom, 2); break;
		case 0xCB08: outputDisassembledInstruction("RRC B", pc, rom, 2); break;
		case 0xCB09: outputDisassembledInstruction("RRC C", pc, rom, 2); break;
		case 0xCB0A: outputDisassembledInstruction("RRC D", pc, rom, 2); break;
		case 0xCB0B: outputDisassembledInstruction("RRC E", pc, rom, 2); break;
		case 0xCB0C: outputDisassembledInstruction("RRC H", pc, rom, 2); break;
		case 0xCB0D: outputDisassembledInstruction("RRC L", pc, rom, 2); break;
		case 0xCB0E: outputDisassembledInstruction("RRC (HL)", pc, rom, 2); break;
		case 0xCB0F: outputDisassembledInstruction("RRC A", pc, rom, 2); break;
		case 0xCB10: outputDisassembledInstruction("RL B", pc, rom, 2); break;
		case 0xCB11: outputDisassembledInstruction("RL C", pc, rom, 2); break;
		case 0xCB12: outputDisassembledInstruction("RL D", pc, rom, 2); break;
		case 0xCB13: outputDisassembledInstruction("RL E", pc, rom, 2); break;
		case 0xCB14: outputDisassembledInstruction("RL H", pc, rom, 2); break;
		case 0xCB15: outputDisassembledInstruction("RL L", pc, rom, 2); break;
		case 0xCB16: outputDisassembledInstruction("RL (HL)", pc, rom, 2); break;
		case 0xCB17: outputDisassembledInstruction("RL A", pc, rom, 2); break;
		case 0xCB18: outputDisassembledInstruction("RR B", pc, rom, 2); break;
		case 0xCB19: outputDisassembledInstruction("RR C", pc, rom, 2); break;
		case 0xCB1A: outputDisassembledInstruction("RR D", pc, rom, 2); break;
		case 0xCB1B: outputDisassembledInstruction("RR E", pc, rom, 2); break;
		case 0xCB1C: outputDisassembledInstruction("RR H", pc, rom, 2); break;
		case 0xCB1D: outputDisassembledInstruction("RR L", pc, rom, 2); break;
		case 0xCB1E: outputDisassembledInstruction("RR (HL)", pc, rom, 2); break;
		case 0xCB1F: outputDisassembledInstruction("RR A", pc, rom, 2); break;
		case 0xCB20: outputDisassembledInstruction("SLA B", pc, rom, 2); break;
		case 0xCB21: outputDisassembledInstruction("SLA C", pc, rom, 2); break;
		case 0xCB22: outputDisassembledInstruction("SLA D", pc, rom, 2); break;
		case 0xCB23: outputDisassembledInstruction("SLA E", pc, rom, 2); break;
		case 0xCB24: outputDisassembledInstruction("SLA H", pc, rom, 2); break;
		case 0xCB25: outputDisassembledInstruction("SLA L", pc, rom, 2); break;
		case 0xCB26: outputDisassembledInstruction("SLA (HL)", pc, rom, 2); break;
		case 0xCB27: outputDisassembledInstruction("SLA A", pc, rom, 2); break;
		case 0xCB28: outputDisassembledInstruction("SRA B", pc, rom, 2); break;
		case 0xCB29: outputDisassembledInstruction("SRA C", pc, rom, 2); break;
		case 0xCB2A: outputDisassembledInstruction("SRA D", pc, rom, 2); break;
		case 0xCB2B: outputDisassembledInstruction("SRA E", pc, rom, 2); break;
		case 0xCB2C: outputDisassembledInstruction("SRA H", pc, rom, 2); break;
		case 0xCB2D: outputDisassembledInstruction("SRA L", pc, rom, 2); break;
		case 0xCB2E: outputDisassembledInstruction("SRA (HL)", pc, rom, 2); break;
		case 0xCB2F: outputDisassembledInstruction("SRA A", pc, rom, 2); break;
		case 0xCB30: outputDisassembledInstruction("SWAP B", pc, rom, 2); break;
		case 0xCB31: outputDisassembledInstruction("SWAP C", pc, rom, 2); break;
		case 0xCB32: outputDisassembledInstruction("SWAP D", pc, rom, 2); break;
		case 0xCB33: outputDisassembledInstruction("SWAP E", pc, rom, 2); break;
		case 0xCB34: outputDisassembledInstruction("SWAP H", pc, rom, 2); break;
		case 0xCB35: outputDisassembledInstruction("SWAP L", pc, rom, 2); break;
		case 0xCB36: outputDisassembledInstruction("SWAP (HL)", pc, rom, 2); break;
		case 0xCB37: outputDisassembledInstruction("SWAP A", pc, rom, 2); break;
		case 0xCB38: outputDisassembledInstruction("SRL B", pc, rom, 2); break;
		case 0xCB39: outputDisassembledInstruction("SRL C", pc, rom, 2); break;
		case 0xCB3A: outputDisassembledInstruction("SRL D", pc, rom, 2); break;
		case 0xCB3B: outputDisassembledInstruction("SRL E", pc, rom, 2); break;
		case 0xCB3C: outputDisassembledInstruction("SRL H", pc, rom, 2); break;
		case 0xCB3D: outputDisassembledInstruction("SRL L", pc, rom, 2); break;
		case 0xCB3E: outputDisassembledInstruction("SRL (HL)", pc, rom, 2); break;
		case 0xCB3F: outputDisassembledInstruction("SRL A", pc, rom, 2); break;
		case 0xCB40: outputDisassembledInstruction("BIT 0, B", pc, rom, 2); break;
		case 0xCB41: outputDisassembledInstruction("BIT 0, C", pc, rom, 2); break;
		case 0xCB42: outputDisassembledInstruction("BIT 0, D", pc, rom, 2); break;
		case 0xCB43: outputDisassembledInstruction("BIT 0, E", pc, rom, 2); break;
		case 0xCB44: outputDisassembledInstruction("BIT 0, H", pc, rom, 2); break;
		case 0xCB45: outputDisassembledInstruction("BIT 0, L", pc, rom, 2); break;
		case 0xCB46: outputDisassembledInstruction("BIT 0, (HL)", pc, rom, 2); break;
		case 0xCB47: outputDisassembledInstruction("BIT 0, A", pc, rom, 2); break;
		case 0xCB48: outputDisassembledInstruction("BIT 1, B", pc, rom, 2); break;
		case 0xCB49: outputDisassembledInstruction("BIT 1, C", pc, rom, 2); break;
		case 0xCB4A: outputDisassembledInstruction("BIT 1, D", pc, rom, 2); break;
		case 0xCB4B: outputDisassembledInstruction("BIT 1, E", pc, rom, 2); break;
		case 0xCB4C: outputDisassembledInstruction("BIT 1, H", pc, rom, 2); break;
		case 0xCB4D: outputDisassembledInstruction("BIT 1, L", pc, rom, 2); break;
		case 0xCB4E: outputDisassembledInstruction("BIT 1, (HL)", pc, rom, 2); break;
		case 0xCB4F: outputDisassembledInstruction("BIT 1, A", pc, rom, 2); break;
		case 0xCB50: outputDisassembledInstruction("BIT 2, B", pc, rom, 2); break;
		case 0xCB51: outputDisassembledInstruction("BIT 2, C", pc, rom, 2); break;
		case 0xCB52: outputDisassembledInstruction("BIT 2, D", pc, rom, 2); break;
		case 0xCB53: outputDisassembledInstruction("BIT 2, E", pc, rom, 2); break;
		case 0xCB54: outputDisassembledInstruction("BIT 2, H", pc, rom, 2); break;
		case 0xCB55: outputDisassembledInstruction("BIT 2, L", pc, rom, 2); break;
		case 0xCB56: outputDisassembledInstruction("BIT 2, (HL)", pc, rom, 2); break;
		case 0xCB57: outputDisassembledInstruction("BIT 2, A", pc, rom, 2); break;
		case 0xCB58: outputDisassembledInstruction("BIT 3, B", pc, rom, 2); break;
		case 0xCB59: outputDisassembledInstruction("BIT 3, C", pc, rom, 2); break;
		case 0xCB5A: outputDisassembledInstruction("BIT 3, D", pc, rom, 2); break;
		case 0xCB5B: outputDisassembledInstruction("BIT 3, E", pc, rom, 2); break;
		case 0xCB5C: outputDisassembledInstruction("BIT 3, H", pc, rom, 2); break;
		case 0xCB5D: outputDisassembledInstruction("BIT 3, L", pc, rom, 2); break;
		case 0xCB5E: outputDisassembledInstruction("BIT 3, (HL)", pc, rom, 2); break;
		case 0xCB5F: outputDisassembledInstruction("BIT 3, A", pc, rom, 2); break;
		case 0xCB60: outputDisassembledInstruction("BIT 4, B", pc, rom, 2); break;
		case 0xCB61: outputDisassembledInstruction("BIT 4, C", pc, rom, 2); break;
		case 0xCB62: outputDisassembledInstruction("BIT 4, D", pc, rom, 2); break;
		case 0xCB63: outputDisassembledInstruction("BIT 4, E", pc, rom, 2); break;
		case 0xCB64: outputDisassembledInstruction("BIT 4, H", pc, rom, 2); break;
		case 0xCB65: outputDisassembledInstruction("BIT 4, L", pc, rom, 2); break;
		case 0xCB66: outputDisassembledInstruction("BIT 4, (HL)", pc, rom, 2); break;
		case 0xCB67: outputDisassembledInstruction("BIT 4, A", pc, rom, 2); break;
		case 0xCB68: outputDisassembledInstruction("BIT 5, B", pc, rom, 2); break;
		case 0xCB69: outputDisassembledInstruction("BIT 5, C", pc, rom, 2); break;
		case 0xCB6A: outputDisassembledInstruction("BIT 5, D", pc, rom, 2); break;
		case 0xCB6B: outputDisassembledInstruction("BIT 5, E", pc, rom, 2); break;
		case 0xCB6C: outputDisassembledInstruction("BIT 5, H", pc, rom, 2); break;
		case 0xCB6D: outputDisassembledInstruction("BIT 5, L", pc, rom, 2); break;
		case 0xCB6E: outputDisassembledInstruction("BIT 5, (HL)", pc, rom, 2); break;
		case 0xCB6F: outputDisassembledInstruction("BIT 5, A", pc, rom, 2); break;
		case 0xCB70: outputDisassembledInstruction("BIT 6, B", pc, rom, 2); break;
		case 0xCB71: outputDisassembledInstruction("BIT 6, C", pc, rom, 2); break;
		case 0xCB72: outputDisassembledInstruction("BIT 6, D", pc, rom, 2); break;
		case 0xCB73: outputDisassembledInstruction("BIT 6, E", pc, rom, 2); break;
		case 0xCB74: outputDisassembledInstruction("BIT 6, H", pc, rom, 2); break;
		case 0xCB75: outputDisassembledInstruction("BIT 6, L", pc, rom, 2); break;
		case 0xCB76: outputDisassembledInstruction("BIT 6, (HL)", pc, rom, 2); break;
		case 0xCB77: outputDisassembledInstruction("BIT 6, A", pc, rom, 2); break;
		case 0xCB78: outputDisassembledInstruction("BIT 7, B", pc, rom, 2); break;
		case 0xCB79: outputDisassembledInstruction("BIT 7, C", pc, rom, 2); break;
		case 0xCB7A: outputDisassembledInstruction("BIT 7, D", pc, rom, 2); break;
		case 0xCB7B: outputDisassembledInstruction("BIT 7, E", pc, rom, 2); break;
		case 0xCB7C: outputDisassembledInstruction("BIT 7, H", pc, rom, 2); break;
		case 0xCB7D: outputDisassembledInstruction("BIT 7, L", pc, rom, 2); break;
		case 0xCB7E: outputDisassembledInstruction("BIT 7, (HL)", pc, rom, 2); break;
		case 0xCB7F: outputDisassembledInstruction("BIT 7, A", pc, rom, 2); break;
		case 0xCB80: outputDisassembledInstruction("RES 0, B", pc, rom, 2); break;
		case 0xCB81: outputDisassembledInstruction("RES 0, C", pc, rom, 2); break;
		case 0xCB82: outputDisassembledInstruction("RES 0, D", pc, rom, 2); break;
		case 0xCB83: outputDisassembledInstruction("RES 0, E", pc, rom, 2); break;
		case 0xCB84: outputDisassembledInstruction("RES 0, H", pc, rom, 2); break;
		case 0xCB85: outputDisassembledInstruction("RES 0, L", pc, rom, 2); break;
		case 0xCB86: outputDisassembledInstruction("RES 0, (HL)", pc, rom, 2); break;
		case 0xCB87: outputDisassembledInstruction("RES 0, A", pc, rom, 2); break;
		case 0xCB88: outputDisassembledInstruction("RES 1, B", pc, rom, 2); break;
		case 0xCB89: outputDisassembledInstruction("RES 1, C", pc, rom, 2); break;
		case 0xCB8A: outputDisassembledInstruction("RES 1, D", pc, rom, 2); break;
		case 0xCB8B: outputDisassembledInstruction("RES 1, E", pc, rom, 2); break;
		case 0xCB8C: outputDisassembledInstruction("RES 1, H", pc, rom, 2); break;
		case 0xCB8D: outputDisassembledInstruction("RES 1, L", pc, rom, 2); break;
		case 0xCB8E: outputDisassembledInstruction("RES 1, (HL)", pc, rom, 2); break;
		case 0xCB8F: outputDisassembledInstruction("RES 1, A", pc, rom, 2); break;
		case 0xCB90: outputDisassembledInstruction("RES 2, B", pc, rom, 2); break;
		case 0xCB91: outputDisassembledInstruction("RES 2, C", pc, rom, 2); break;
		case 0xCB92: outputDisassembledInstruction("RES 2, D", pc, rom, 2); break;
		case 0xCB93: outputDisassembledInstruction("RES 2, E", pc, rom, 2); break;
		case 0xCB94: outputDisassembledInstruction("RES 2, H", pc, rom, 2); break;
		case 0xCB95: outputDisassembledInstruction("RES 2, L", pc, rom, 2); break;
		case 0xCB96: outputDisassembledInstruction("RES 2, (HL)", pc, rom, 2); break;
		case 0xCB97: outputDisassembledInstruction("RES 2, A", pc, rom, 2); break;
		case 0xCB98: outputDisassembledInstruction("RES 3, B", pc, rom, 2); break;
		case 0xCB99: outputDisassembledInstruction("RES 3, C", pc, rom, 2); break;
		case 0xCB9A: outputDisassembledInstruction("RES 3, D", pc, rom, 2); break;
		case 0xCB9B: outputDisassembledInstruction("RES 3, E", pc, rom, 2); break;
		case 0xCB9C: outputDisassembledInstruction("RES 3, H", pc, rom, 2); break;
		case 0xCB9D: outputDisassembledInstruction("RES 3, L", pc, rom, 2); break;
		case 0xCB9E: outputDisassembledInstruction("RES 3, (HL)", pc, rom, 2); break;
		case 0xCB9F: outputDisassembledInstruction("RES 3, A", pc, rom, 2); break;
		case 0xCBA0: outputDisassembledInstruction("RES 4, B", pc, rom, 2); break;
		case 0xCBA1: outputDisassembledInstruction("RES 4, C", pc, rom, 2); break;
		case 0xCBA2: outputDisassembledInstruction("RES 4, D", pc, rom, 2); break;
		case 0xCBA3: outputDisassembledInstruction("RES 4, E", pc, rom, 2); break;
		case 0xCBA4: outputDisassembledInstruction("RES 4, H", pc, rom, 2); break;
		case 0xCBA5: outputDisassembledInstruction("RES 4, L", pc, rom, 2); break;
		case 0xCBA6: outputDisassembledInstruction("RES 4, (HL)", pc, rom, 2); break;
		case 0xCBA7: outputDisassembledInstruction("RES 4, A", pc, rom, 2); break;
		case 0xCBA8: outputDisassembledInstruction("RES 5, B", pc, rom, 2); break;
		case 0xCBA9: outputDisassembledInstruction("RES 5, C", pc, rom, 2); break;
		case 0xCBAA: outputDisassembledInstruction("RES 5, D", pc, rom, 2); break;
		case 0xCBAB: outputDisassembledInstruction("RES 5, E", pc, rom, 2); break;
		case 0xCBAC: outputDisassembledInstruction("RES 5, H", pc, rom, 2); break;
		case 0xCBAD: outputDisassembledInstruction("RES 5, L", pc, rom, 2); break;
		case 0xCBAE: outputDisassembledInstruction("RES 5, (HL)", pc, rom, 2); break;
		case 0xCBAF: outputDisassembledInstruction("RES 5, A", pc, rom, 2); break;
		case 0xCBB0: outputDisassembledInstruction("RES 6, B", pc, rom, 2); break;
		case 0xCBB1: outputDisassembledInstruction("RES 6, C", pc, rom, 2); break;
		case 0xCBB2: outputDisassembledInstruction("RES 6, D", pc, rom, 2); break;
		case 0xCBB3: outputDisassembledInstruction("RES 6, E", pc, rom, 2); break;
		case 0xCBB4: outputDisassembledInstruction("RES 6, H", pc, rom, 2); break;
		case 0xCBB5: outputDisassembledInstruction("RES 6, L", pc, rom, 2); break;
		case 0xCBB6: outputDisassembledInstruction("RES 6, (HL)", pc, rom, 2); break;
		case 0xCBB7: outputDisassembledInstruction("RES 6, A", pc, rom, 2); break;
		case 0xCBB8: outputDisassembledInstruction("RES 7, B", pc, rom, 2); break;
		case 0xCBB9: outputDisassembledInstruction("RES 7, C", pc, rom, 2); break;
		case 0xCBBA: outputDisassembledInstruction("RES 7, D", pc, rom, 2); break;
		case 0xCBBB: outputDisassembledInstruction("RES 7, E", pc, rom, 2); break;
		case 0xCBBC: outputDisassembledInstruction("RES 7, H", pc, rom, 2); break;
		case 0xCBBD: outputDisassembledInstruction("RES 7, L", pc, rom, 2); break;
		case 0xCBBE: outputDisassembledInstruction("RES 7, (HL)", pc, rom, 2); break;
		case 0xCBBF: outputDisassembledInstruction("RES 7, A", pc, rom, 2); break;
		case 0xCBC0: outputDisassembledInstruction("SET 0, B", pc, rom, 2); break;
		case 0xCBC1: outputDisassembledInstruction("SET 0, C", pc, rom, 2); break;
		case 0xCBC2: outputDisassembledInstruction("SET 0, D", pc, rom, 2); break;
		case 0xCBC3: outputDisassembledInstruction("SET 0, E", pc, rom, 2); break;
		case 0xCBC4: outputDisassembledInstruction("SET 0, H", pc, rom, 2); break;
		case 0xCBC5: outputDisassembledInstruction("SET 0, L", pc, rom, 2); break;
		case 0xCBC6: outputDisassembledInstruction("SET 0, (HL)", pc, rom, 2); break;
		case 0xCBC7: outputDisassembledInstruction("SET 0, A", pc, rom, 2); break;
		case 0xCBC8: outputDisassembledInstruction("SET 1, B", pc, rom, 2); break;
		case 0xCBC9: outputDisassembledInstruction("SET 1, C", pc, rom, 2); break;
		case 0xCBCA: outputDisassembledInstruction("SET 1, D", pc, rom, 2); break;
		case 0xCBCB: outputDisassembledInstruction("SET 1, E", pc, rom, 2); break;
		case 0xCBCC: outputDisassembledInstruction("SET 1, H", pc, rom, 2); break;
		case 0xCBCD: outputDisassembledInstruction("SET 1, L", pc, rom, 2); break;
		case 0xCBCE: outputDisassembledInstruction("SET 1, (HL)", pc, rom, 2); break;
		case 0xCBCF: outputDisassembledInstruction("SET 1, A", pc, rom, 2); break;
		case 0xCBD0: outputDisassembledInstruction("SET 2, B", pc, rom, 2); break;
		case 0xCBD1: outputDisassembledInstruction("SET 2, C", pc, rom, 2); break;
		case 0xCBD2: outputDisassembledInstruction("SET 2, D", pc, rom, 2); break;
		case 0xCBD3: outputDisassembledInstruction("SET 2, E", pc, rom, 2); break;
		case 0xCBD4: outputDisassembledInstruction("SET 2, H", pc, rom, 2); break;
		case 0xCBD5: outputDisassembledInstruction("SET 2, L", pc, rom, 2); break;
		case 0xCBD6: outputDisassembledInstruction("SET 2, (HL)", pc, rom, 2); break;
		case 0xCBD7: outputDisassembledInstruction("SET 2, A", pc, rom, 2); break;
		case 0xCBD8: outputDisassembledInstruction("SET 3, B", pc, rom, 2); break;
		case 0xCBD9: outputDisassembledInstruction("SET 3, C", pc, rom, 2); break;
		case 0xCBDA: outputDisassembledInstruction("SET 3, D", pc, rom, 2); break;
		case 0xCBDB: outputDisassembledInstruction("SET 3, E", pc, rom, 2); break;
		case 0xCBDC: outputDisassembledInstruction("SET 3, H", pc, rom, 2); break;
		case 0xCBDD: outputDisassembledInstruction("SET 3, L", pc, rom, 2); break;
		case 0xCBDE: outputDisassembledInstruction("SET 3, (HL)", pc, rom, 2); break;
		case 0xCBDF: outputDisassembledInstruction("SET 3, A", pc, rom, 2); break;
		case 0xCBE0: outputDisassembledInstruction("SET 4, B", pc, rom, 2); break;
		case 0xCBE1: outputDisassembledInstruction("SET 4, C", pc, rom, 2); break;
		case 0xCBE2: outputDisassembledInstruction("SET 4, D", pc, rom, 2); break;
		case 0xCBE3: outputDisassembledInstruction("SET 4, E", pc, rom, 2); break;
		case 0xCBE4: outputDisassembledInstruction("SET 4, H", pc, rom, 2); break;
		case 0xCBE5: outputDisassembledInstruction("SET 4, L", pc, rom, 2); break;
		case 0xCBE6: outputDisassembledInstruction("SET 4, (HL)", pc, rom, 2); break;
		case 0xCBE7: outputDisassembledInstruction("SET 4, A", pc, rom, 2); break;
		case 0xCBE8: outputDisassembledInstruction("SET 5, B", pc, rom, 2); break;
		case 0xCBE9: outputDisassembledInstruction("SET 5, C", pc, rom, 2); break;
		case 0xCBEA: outputDisassembledInstruction("SET 5, D", pc, rom, 2); break;
		case 0xCBEB: outputDisassembledInstruction("SET 5, E", pc, rom, 2); break;
		case 0xCBEC: outputDisassembledInstruction("SET 5, H", pc, rom, 2); break;
		case 0xCBED: outputDisassembledInstruction("SET 5, L", pc, rom, 2); break;
		case 0xCBEE: outputDisassembledInstruction("SET 5, (HL)", pc, rom, 2); break;
		case 0xCBEF: outputDisassembledInstruction("SET 5, A", pc, rom, 2); break;
		case 0xCBF0: outputDisassembledInstruction("SET 6, B", pc, rom, 2); break;
		case 0xCBF1: outputDisassembledInstruction("SET 6, C", pc, rom, 2); break;
		case 0xCBF2: outputDisassembledInstruction("SET 6, D", pc, rom, 2); break;
		case 0xCBF3: outputDisassembledInstruction("SET 6, E", pc, rom, 2); break;
		case 0xCBF4: outputDisassembledInstruction("SET 6, H", pc, rom, 2); break;
		case 0xCBF5: outputDisassembledInstruction("SET 6, L", pc, rom, 2); break;
		case 0xCBF6: outputDisassembledInstruction("SET 6, (HL)", pc, rom, 2); break;
		case 0xCBF7: outputDisassembledInstruction("SET 6, A", pc, rom, 2); break;
		case 0xCBF8: outputDisassembledInstruction("SET 7, B", pc, rom, 2); break;
		case 0xCBF9: outputDisassembledInstruction("SET 7, C", pc, rom, 2); break;
		case 0xCBFA: outputDisassembledInstruction("SET 7, D", pc, rom, 2); break;
		case 0xCBFB: outputDisassembledInstruction("SET 7, E", pc, rom, 2); break;
		case 0xCBFC: outputDisassembledInstruction("SET 7, H", pc, rom, 2); break;
		case 0xCBFD: outputDisassembledInstruction("SET 7, L", pc, rom, 2); break;
		case 0xCBFE: outputDisassembledInstruction("SET 7, (HL)", pc, rom, 2); break;
		case 0xCBFF: outputDisassembledInstruction("SET 7, A", pc, rom, 2); break;
	}
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