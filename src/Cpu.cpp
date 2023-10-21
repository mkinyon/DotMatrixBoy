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

		// PLACEHOLDER 
		case 0xCB: outputDisassembledInstruction("16-BIT OPCODE", pc, rom, 2); opBytes = 2; break;

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