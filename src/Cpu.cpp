#include "Cpu.h"
#include "GameBoy.h"

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
	struct cpuFlags flags;
	uint8_t	int_enable;
};

Cpu::cpuState state;

Cpu::Cpu() {}
Cpu::~Cpu() {}

void Cpu::Clock(GameBoy& gb)
{
	uint8_t* opcode = &gb.ReadFromMemoryMap(state.pc);

	Disassemble(opcode, state.pc);

	state.pc += 1;  //for the opcode 

	switch (*opcode)
	{


		/********************************************************************************************
			Misc / Control Instructions
		*********************************************************************************************/
		
		// "NOP" B:1 C:4 FLAGS: - - - -
		case 0x00: break;

		// "STOP n8" B:2 C:4 FLAGS: - - - -
		case 0x10: { unimplementedInstruction(state, *opcode); break; }

		// "HALT" B:1 C:4 FLAGS: - - - -
		case 0x76: { unimplementedInstruction(state, *opcode); break; }

		// "PREFIX" B:1 C:4 FLAGS: - - - -
		case 0xCB: { unimplementedInstruction(state, *opcode); break; }

		// "DI" B:1 C:4 FLAGS: - - - -
		case 0xF3:
		{
			state.int_enable = 0;
			break;
		}

		// "EI" B:1 C:4 FLAGS: - - - -
		case 0xFB: { unimplementedInstruction(state, *opcode); break; }


		/********************************************************************************************
			Jumps/Calls
		*********************************************************************************************/

		// "JR e8" B:2 C:12 FLAGS: - - - -
		case 0x18: { unimplementedInstruction(state, *opcode); break; }

		// "JR NZ e8" B:2 C:128 FLAGS: - - - -
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
			break;
		}

		// "JR Z e8" B:2 C:128 FLAGS: - - - -
		case 0x28: { unimplementedInstruction(state, *opcode); break; }
		
		// "JR NC e8" B:2 C:128 FLAGS: - - - -
		case 0x30: { unimplementedInstruction(state, *opcode); break; }

		// "JR C e8" B:2 C:128 FLAGS: - - - -
		case 0x38:
		{
			// note: "s8" in the description refers to a signed char
			int8_t offset = opcode[1];
			if (state.flags.c == 1)
			{
				state.pc += offset;
			}
			else
			{
				state.pc++;
			}
			break;
		}

		// "RET NZ" B:1 C:208 FLAGS: - - - -
		case 0xC0: { unimplementedInstruction(state, *opcode); break; }

		// "JP NZ a16" B:3 C:1612 FLAGS: - - - -
		case 0xC2: { unimplementedInstruction(state, *opcode); break; }

		// "JP a16" B:3 C:16 FLAGS: - - - -
		case 0xC3:
		{
			uint16_t offset = (opcode[2] << 8) | (opcode[1]);
			state.pc = offset;
			break;
		}

		// "CALL NZ a16" B:3 C:2412 FLAGS: - - - -
		case 0xC4: { unimplementedInstruction(state, *opcode); break; }
		
		// "RST $00" B:1 C:16 FLAGS: - - - -
		case 0xC7: { unimplementedInstruction(state, *opcode); break; }
		
		// "RET Z" B:1 C:208 FLAGS: - - - -
		case 0xC8: { unimplementedInstruction(state, *opcode); break; }
		
		// "RET" B:1 C:16 FLAGS: - - - -
		case 0xC9: { unimplementedInstruction(state, *opcode); break; }
		
		// "JP Z a16" B:3 C:1612 FLAGS: - - - -
		case 0xCA: { unimplementedInstruction(state, *opcode); break; }
		
		// "CALL Z a16" B:3 C:2412 FLAGS: - - - -
		case 0xCC: { unimplementedInstruction(state, *opcode); break; }
		
		// "CALL a16" B:3 C:24 FLAGS: - - - -
		case 0xCD: { unimplementedInstruction(state, *opcode); break; }
		
		// "RST $08" B:1 C:16 FLAGS: - - - -
		case 0xCF: { unimplementedInstruction(state, *opcode); break; }
		
		// "RET NC" B:1 C:208 FLAGS: - - - -
		case 0xD0: { unimplementedInstruction(state, *opcode); break; }
		
		// "JP NC a16" B:3 C:1612 FLAGS: - - - -
		case 0xD2: { unimplementedInstruction(state, *opcode); break; }
		
		// "CALL NC a16" B:3 C:2412 FLAGS: - - - -
		case 0xD4: { unimplementedInstruction(state, *opcode); break; }
		
		// "RST $10" B:1 C:16 FLAGS: - - - -
		case 0xD7: { unimplementedInstruction(state, *opcode); break; }
		
		// "RET C" B:1 C:208 FLAGS: - - - -
		case 0xD8: { unimplementedInstruction(state, *opcode); break; }
		
		// "RETI" B:1 C:16 FLAGS: - - - -
		case 0xD9: { unimplementedInstruction(state, *opcode); break; }
		
		// "JP C a16" B:3 C:1612 FLAGS: - - - -
		case 0xDA: { unimplementedInstruction(state, *opcode); break; }
		
		// "CALL C a16" B:3 C:2412 FLAGS: - - - -
		case 0xDC: { unimplementedInstruction(state, *opcode); break; }
		
		// "RST $18" B:1 C:16 FLAGS: - - - -
		case 0xDF: { unimplementedInstruction(state, *opcode); break; }
		
		// "RST $20" B:1 C:16 FLAGS: - - - -
		case 0xE7: { unimplementedInstruction(state, *opcode); break; }
		
		// "JP HL" B:1 C:4 FLAGS: - - - -
		case 0xE9: { unimplementedInstruction(state, *opcode); break; }
		
		// "RST $28" B:1 C:16 FLAGS: - - - -
		case 0xEF: { unimplementedInstruction(state, *opcode); break; }
		
		// "RST $30" B:1 C:16 FLAGS: - - - -
		case 0xF7: { unimplementedInstruction(state, *opcode); break; }

		// "RST $38" B:1 C:16 FLAGS: - - - -
		case 0xFF:
		{
			state.sp = state.pc;
			state.pc = 0x00 + 0x38;

			//gb.WriteToMemoryMap(state.sp, (state.pc) >> 8);
			//gb.WriteToMemoryMap(state.sp, (state.pc) & 0xff);
			state.pc = opcode[0] ^ 0xC7;
			break;
		}


		/********************************************************************************************
			8-bit Load Instructions 
		*********************************************************************************************/
		
		// "LD [BC] A" B:1 C:8 FLAGS: - - - -
		case 0x02:
		{
			uint16_t offset = (state.b << 8) | state.c;
			gb.WriteToMemoryMap(offset, state.a);
			break;
		}

		// "LD B n8" B:2 C:8 FLAGS: - - - -
		case 0x06:
		{
			state.b = opcode[1];
			state.pc++;
			break;
		}

		// "LD A [BC]" B:1 C:8 FLAGS: - - - -
		case 0x0A: { unimplementedInstruction(state, *opcode); break; }
		
		// "LD C n8" B:2 C:8 FLAGS: - - - -
		case 0x0E:
		{
			state.c = opcode[1];
			state.pc++;
			break;
		}

		// "LD [DE] A" B:1 C:8 FLAGS: - - - -
		case 0x12: { unimplementedInstruction(state, *opcode); break; }

		// "LD D n8" B:2 C:8 FLAGS: - - - -
		case 0x16: { unimplementedInstruction(state, *opcode); break; }
		
		// "LD A [DE]" B:1 C:8 FLAGS: - - - -
		case 0x1A: { unimplementedInstruction(state, *opcode); break; }
		
		// "LD E n8" B:2 C:8 FLAGS: - - - -
		case 0x1E: { unimplementedInstruction(state, *opcode); break; }

		// "LD [HL] A" B:1 C:8 FLAGS: - - - -
		case 0x22: { unimplementedInstruction(state, *opcode); break; }
		
		// "LD H n8" B:2 C:8 FLAGS: - - - -
		case 0x26: { unimplementedInstruction(state, *opcode); break; }
		
		// "LD A [HL]" B:1 C:8 FLAGS: - - - -
		case 0x2A: { unimplementedInstruction(state, *opcode); break; }
		
		// "LD L n8" B:2 C:8 FLAGS: - - - -
		case 0x2E: { unimplementedInstruction(state, *opcode); break; }

		// "LD [HL] A" B:1 C:8 FLAGS: - - - -
		case 0x32:
		{
			uint16_t offset = (opcode[2] << 8) | (opcode[1]);
			gb.WriteToMemoryMap(offset, state.a);
			break;
		}

		// "LD [HL] n8" B:2 C:12 FLAGS: - - - -
		case 0x36:
		{
			state.h = opcode[1];
			state.pc++;
			break;
		}

		// "LD A [HL]" B:1 C:8 FLAGS: - - - -
		case 0x3A: { unimplementedInstruction(state, *opcode); break; }

		// "LD A n8" B:2 C:8 FLAGS: - - - -
		case 0x3E:
		{
			state.a = opcode[1];
			state.pc++;
			break;
		}

		// "LD B B" B:1 C:4 FLAGS: - - - -
		case 0x40: { unimplementedInstruction(state, *opcode); break; }

		// "LD B C" B:1 C:4 FLAGS: - - - -
		case 0x41: { unimplementedInstruction(state, *opcode); break; }

		// "LD B D" B:1 C:4 FLAGS: - - - -
		case 0x42: { unimplementedInstruction(state, *opcode); break; }

		// "LD B E" B:1 C:4 FLAGS: - - - -
		case 0x43: { unimplementedInstruction(state, *opcode); break; }

		// "LD B H" B:1 C:4 FLAGS: - - - -
		case 0x44: { unimplementedInstruction(state, *opcode); break; }

		// "LD B L" B:1 C:4 FLAGS: - - - -
		case 0x45: { unimplementedInstruction(state, *opcode); break; }

		// "LD B [HL]" B:1 C:8 FLAGS: - - - -
		case 0x46: { unimplementedInstruction(state, *opcode); break; }

		// "LD B A" B:1 C:4 FLAGS: - - - -
		case 0x47: { unimplementedInstruction(state, *opcode); break; }

		// "LD C B" B:1 C:4 FLAGS: - - - -
		case 0x48: { unimplementedInstruction(state, *opcode); break; }

		// "LD C C" B:1 C:4 FLAGS: - - - -
		case 0x49: { unimplementedInstruction(state, *opcode); break; }

		// "LD C D" B:1 C:4 FLAGS: - - - -
		case 0x4A: { unimplementedInstruction(state, *opcode); break; }

		// "LD C E" B:1 C:4 FLAGS: - - - -
		case 0x4B: { unimplementedInstruction(state, *opcode); break; }

		// "LD C H" B:1 C:4 FLAGS: - - - -
		case 0x4C: { unimplementedInstruction(state, *opcode); break; }

		// "LD C L" B:1 C:4 FLAGS: - - - -
		case 0x4D: { unimplementedInstruction(state, *opcode); break; }

		// "LD C [HL]" B:1 C:8 FLAGS: - - - -
		case 0x4E: { unimplementedInstruction(state, *opcode); break; }

		// "LD C A" B:1 C:4 FLAGS: - - - -
		case 0x4F: { unimplementedInstruction(state, *opcode); break; }

		// "LD D B" B:1 C:4 FLAGS: - - - -
		case 0x50: { unimplementedInstruction(state, *opcode); break; }

		// "LD D C" B:1 C:4 FLAGS: - - - -
		case 0x51: { unimplementedInstruction(state, *opcode); break; }

		// "LD D D" B:1 C:4 FLAGS: - - - -
		case 0x52: { unimplementedInstruction(state, *opcode); break; }

		// "LD D E" B:1 C:4 FLAGS: - - - -
		case 0x53: { unimplementedInstruction(state, *opcode); break; }

		// "LD D H" B:1 C:4 FLAGS: - - - -
		case 0x54: { unimplementedInstruction(state, *opcode); break; }

		// "LD D L" B:1 C:4 FLAGS: - - - -
		case 0x55: { unimplementedInstruction(state, *opcode); break; }

		// "LD D [HL]" B:1 C:8 FLAGS: - - - -
		case 0x56: { unimplementedInstruction(state, *opcode); break; }

		// "LD D A" B:1 C:4 FLAGS: - - - -
		case 0x57: { unimplementedInstruction(state, *opcode); break; }

		// "LD E B" B:1 C:4 FLAGS: - - - -
		case 0x58: { unimplementedInstruction(state, *opcode); break; }

		// "LD E C" B:1 C:4 FLAGS: - - - -
		case 0x59: { unimplementedInstruction(state, *opcode); break; }

		// "LD E D" B:1 C:4 FLAGS: - - - -
		case 0x5A: { unimplementedInstruction(state, *opcode); break; }

		// "LD E E" B:1 C:4 FLAGS: - - - -
		case 0x5B: { unimplementedInstruction(state, *opcode); break; }

		// "LD E H" B:1 C:4 FLAGS: - - - -
		case 0x5C: { unimplementedInstruction(state, *opcode); break; }

		// "LD E L" B:1 C:4 FLAGS: - - - -
		case 0x5D: { unimplementedInstruction(state, *opcode); break; }

		// "LD E [HL]" B:1 C:8 FLAGS: - - - -
		case 0x5E: { unimplementedInstruction(state, *opcode); break; }

		// "LD E A" B:1 C:4 FLAGS: - - - -
		case 0x5F: { unimplementedInstruction(state, *opcode); break; }

		// "LD H B" B:1 C:4 FLAGS: - - - -
		case 0x60: { unimplementedInstruction(state, *opcode); break; }

		// "LD H C" B:1 C:4 FLAGS: - - - -
		case 0x61: { unimplementedInstruction(state, *opcode); break; }

		// "LD H D" B:1 C:4 FLAGS: - - - -
		case 0x62: { unimplementedInstruction(state, *opcode); break; }

		// "LD H E" B:1 C:4 FLAGS: - - - -
		case 0x63: { unimplementedInstruction(state, *opcode); break; }

		// "LD H H" B:1 C:4 FLAGS: - - - -
		case 0x64: { unimplementedInstruction(state, *opcode); break; }

		// "LD H L" B:1 C:4 FLAGS: - - - -
		case 0x65: { unimplementedInstruction(state, *opcode); break; }

		// "LD H [HL]" B:1 C:8 FLAGS: - - - -
		case 0x66: { unimplementedInstruction(state, *opcode); break; }

		// "LD H A" B:1 C:4 FLAGS: - - - -
		case 0x67: { unimplementedInstruction(state, *opcode); break; }

		// "LD L B" B:1 C:4 FLAGS: - - - -
		case 0x68: { unimplementedInstruction(state, *opcode); break; }

		// "LD L C" B:1 C:4 FLAGS: - - - -
		case 0x69: { unimplementedInstruction(state, *opcode); break; }

		// "LD L D" B:1 C:4 FLAGS: - - - -
		case 0x6A: { unimplementedInstruction(state, *opcode); break; }

		// "LD L E" B:1 C:4 FLAGS: - - - -
		case 0x6B: { unimplementedInstruction(state, *opcode); break; }

		// "LD L H" B:1 C:4 FLAGS: - - - -
		case 0x6C: { unimplementedInstruction(state, *opcode); break; }

		// "LD L L" B:1 C:4 FLAGS: - - - -
		case 0x6D: { unimplementedInstruction(state, *opcode); break; }

		// "LD L [HL]" B:1 C:8 FLAGS: - - - -
		case 0x6E: { unimplementedInstruction(state, *opcode); break; }

		// "LD L A" B:1 C:4 FLAGS: - - - -
		case 0x6F: { unimplementedInstruction(state, *opcode); break; }

		// "LD [HL] B" B:1 C:8 FLAGS: - - - -
		case 0x70: { unimplementedInstruction(state, *opcode); break; }

		// "LD [HL] C" B:1 C:8 FLAGS: - - - -
		case 0x71: { unimplementedInstruction(state, *opcode); break; }

		// "LD [HL] D" B:1 C:8 FLAGS: - - - -
		case 0x72: { unimplementedInstruction(state, *opcode); break; }

		// "LD [HL] E" B:1 C:8 FLAGS: - - - -
		case 0x73: { unimplementedInstruction(state, *opcode); break; }

		// "LD [HL] H" B:1 C:8 FLAGS: - - - -
		case 0x74: { unimplementedInstruction(state, *opcode); break; }

		// "LD [HL] L" B:1 C:8 FLAGS: - - - -
		case 0x75: { unimplementedInstruction(state, *opcode); break; }

		// "LD [HL] A" B:1 C:8 FLAGS: - - - -
		case 0x77: { unimplementedInstruction(state, *opcode); break; }

		// "LD A B" B:1 C:4 FLAGS: - - - -
		case 0x78: { unimplementedInstruction(state, *opcode); break; }

		// "LD A C" B:1 C:4 FLAGS: - - - -
		case 0x79: { unimplementedInstruction(state, *opcode); break; }

		// "LD A D" B:1 C:4 FLAGS: - - - -
		case 0x7A: { unimplementedInstruction(state, *opcode); break; }

		// "LD A E" B:1 C:4 FLAGS: - - - -
		case 0x7B: { unimplementedInstruction(state, *opcode); break; }

		// "LD A H" B:1 C:4 FLAGS: - - - -
		case 0x7C: { unimplementedInstruction(state, *opcode); break; }

		// "LD A L" B:1 C:4 FLAGS: - - - -
		case 0x7D: { unimplementedInstruction(state, *opcode); break; }

		// "LD A [HL]" B:1 C:8 FLAGS: - - - -
		case 0x7E: { unimplementedInstruction(state, *opcode); break; }

		// "LD A A" B:1 C:4 FLAGS: - - - -
		case 0x7F:
		{
			state.a = state.a;
			break;
		}

		// "LD C n8" B:2 C:8 FLAGS: - - - -
		case 0xE0:
		{
			uint8_t offset = opcode[1];
			gb.WriteToMemoryMap(0xFF00 + offset, state.a);
			state.pc++;
			break;
		}

		// "LD [C] A" B:1 C:8 FLAGS: - - - -
		case 0xE2: { unimplementedInstruction(state, *opcode); break; }

		// "LD [a16] A" B:3 C:16 FLAGS: - - - -
		case 0xEA:
		{
			gb.WriteToMemoryMap((opcode[2] << 8) | (opcode[1]), state.a);
			state.pc = state.pc + 2;
			break;
		}
		
		// "LDH A [a8]" B:2 C:12 FLAGS: - - - -
		case 0xF0:
		{
			state.a = gb.ReadFromMemoryMap(0xFF00 + opcode[1]);
			state.pc++;
			break;
		}

		// "LD A [C]" B:1 C:8 FLAGS: - - - -
		case 0xF2: { unimplementedInstruction(state, *opcode); break; }

		// "LD A [a16]" B:3 C:16 FLAGS: - - - -
		case 0xFA: { unimplementedInstruction(state, *opcode); break; }


		/********************************************************************************************
			16-bit Load Instructions
		*********************************************************************************************/

		// "LD BC n16" B:3 C:12 FLAGS: - - - -
		case 0x01:
		{
			state.c = opcode[1];
			state.b = opcode[2];
			state.pc += 2;
			break;
		}

		// "LD [a16] SP" B:3 C:20 FLAGS: - - - -
		case 0x08: { unimplementedInstruction(state, *opcode); break; }

		// "LD DE n16" B:3 C:12 FLAGS: - - - -
		case 0x11: { unimplementedInstruction(state, *opcode); break; }
		
		// "LD HL n16" B:3 C:12 FLAGS: - - - -
		case 0x21:
		{
			state.h = opcode[1];
			state.l = opcode[2];
			state.pc += 2;
			break;
		}

		// "LD SP n16" B:3 C:12 FLAGS: - - - -
		case 0x31: { unimplementedInstruction(state, *opcode); break; }
				 
		// "POP BC" B:1 C:12 FLAGS: - - - -
		case 0xC1: { unimplementedInstruction(state, *opcode); break; }
		
		// "PUSH BC" B:1 C:16 FLAGS: - - - -
		case 0xC5: { unimplementedInstruction(state, *opcode); break; }
		
		// "POP DE" B:1 C:12 FLAGS: - - - -
		case 0xD1: { unimplementedInstruction(state, *opcode); break; }
		
		// "PUSH DE" B:1 C:16 FLAGS: - - - -
		case 0xD5: { unimplementedInstruction(state, *opcode); break; }
		
		// "POP HL" B:1 C:12 FLAGS: - - - -
		case 0xE1: { unimplementedInstruction(state, *opcode); break; }
		
		// "PUSH HL" B:1 C:16 FLAGS: - - - -
		case 0xE5: { unimplementedInstruction(state, *opcode); break; }
		
		// "POP AF" B:1 C:12 FLAGS: Z N H C
		case 0xF1: { unimplementedInstruction(state, *opcode); break; }
		
		// "PUSH AF" B:1 C:16 FLAGS: - - - -
		case 0xF5: { unimplementedInstruction(state, *opcode); break; }
		
		// "LD HL SP e8" B:2 C:12 FLAGS: 0 0 H C
		case 0xF8: { unimplementedInstruction(state, *opcode); break; }

		// "LD SP HL" B:1 C:8 FLAGS: - - - -
		case 0xF9:
		{
			state.sp = (state.l << 8) | (state.h);
			break;
		}


		/********************************************************************************************
			8-bit Arithmetic/Logical Instructions
		*********************************************************************************************/

		// "INC B" B:1 C:4 FLAGS: Z 0 H -
		case 0x04: { unimplementedInstruction(state, *opcode); break; }

		// "DEC B" B:1 C:4 FLAGS: Z 1 H -
		case 0x05:
		{
			state.b--;

			if (state.b == 0)
				state.flags.z = 1;

			state.flags.n = 1;
			break;
		}

		// "INC C" B:1 C:4 FLAGS: Z 0 H -
		case 0x0C: { unimplementedInstruction(state, *opcode); break; }

		// "DEC C" B:1 C:4 FLAGS: Z 1 H -
		case 0x0D: 
		{ 
			state.c--;

			if (state.c == 0) 
				state.flags.z = 1;

			state.flags.n = 1;
			break; 
		}

		// "INC D" B:1 C:4 FLAGS: Z 0 H -
		case 0x14: { unimplementedInstruction(state, *opcode); break; }

		// "DEC D" B:1 C:4 FLAGS: Z 1 H -
		case 0x15: { unimplementedInstruction(state, *opcode); break; }

		// "INC E" B:1 C:4 FLAGS: Z 0 H -
		case 0x1C: { unimplementedInstruction(state, *opcode); break; }

		// "DEC E" B:1 C:4 FLAGS: Z 1 H -
		case 0x1D: { unimplementedInstruction(state, *opcode); break; }

		// "INC H" B:1 C:4 FLAGS: Z 0 H -
		case 0x24: { unimplementedInstruction(state, *opcode); break; }

		// "DEC H" B:1 C:4 FLAGS: Z 1 H -
		case 0x25: { unimplementedInstruction(state, *opcode); break; }
		
		// "DAA" B:1 C:4 FLAGS: Z - 0 C
		case 0x27: { unimplementedInstruction(state, *opcode); break; }

		// "INC L" B:1 C:4 FLAGS: Z 0 H -
		case 0x2C: { unimplementedInstruction(state, *opcode); break; }

		// "DEC L" B:1 C:4 FLAGS: Z 1 H -
		case 0x2D: { unimplementedInstruction(state, *opcode); break; }

		// "CPL" B:1 C:4 FLAGS: - 1 1 -
		case 0x2F: { unimplementedInstruction(state, *opcode); break; }

		// "INC [HL]" B:1 C:12 FLAGS: Z 0 H -
		case 0x34: { unimplementedInstruction(state, *opcode); break; }

		// "DEC [HL]" B:1 C:12 FLAGS: Z 1 H -
		case 0x35: { unimplementedInstruction(state, *opcode); break; }
		
		// "SCF" B:1 C:4 FLAGS: - 0 0 1
		case 0x37: { unimplementedInstruction(state, *opcode); break; }

		// "INC A" B:1 C:4 FLAGS: Z 0 H -
		case 0x3C: { unimplementedInstruction(state, *opcode); break; }

		// "DEC A" B:1 C:4 FLAGS: Z 1 H -
		case 0x3D: { unimplementedInstruction(state, *opcode); break; }

		// "CCF" B:1 C:4 FLAGS: - 0 0 C
		case 0x3F: { unimplementedInstruction(state, *opcode); break; }

		// "ADD A B" B:1 C:4 FLAGS: Z 0 H C
		case 0x80: { unimplementedInstruction(state, *opcode); break; }

		// "ADD A C" B:1 C:4 FLAGS: Z 0 H C
		case 0x81: { unimplementedInstruction(state, *opcode); break; }

		// "ADD A D" B:1 C:4 FLAGS: Z 0 H C
		case 0x82: { unimplementedInstruction(state, *opcode); break; }

		// "ADD A E" B:1 C:4 FLAGS: Z 0 H C
		case 0x83: { unimplementedInstruction(state, *opcode); break; }

		// "ADD A H" B:1 C:4 FLAGS: Z 0 H C
		case 0x84: { unimplementedInstruction(state, *opcode); break; }

		// "ADD A L" B:1 C:4 FLAGS: Z 0 H C
		case 0x85: { unimplementedInstruction(state, *opcode); break; }

		// "ADD A [HL]" B:1 C:8 FLAGS: Z 0 H C
		case 0x86: { unimplementedInstruction(state, *opcode); break; }

		// "ADD A A" B:1 C:4 FLAGS: Z 0 H C
		case 0x87: { unimplementedInstruction(state, *opcode); break; }

		// "ADC A B" B:1 C:4 FLAGS: Z 0 H C
		case 0x88: { unimplementedInstruction(state, *opcode); break; }

		// "ADC A C" B:1 C:4 FLAGS: Z 0 H C
		case 0x89: { unimplementedInstruction(state, *opcode); break; }

		// "ADC A D" B:1 C:4 FLAGS: Z 0 H C
		case 0x8A: { unimplementedInstruction(state, *opcode); break; }

		// "ADC A E" B:1 C:4 FLAGS: Z 0 H C
		case 0x8B: { unimplementedInstruction(state, *opcode); break; }

		// "ADC A H" B:1 C:4 FLAGS: Z 0 H C
		case 0x8C: { unimplementedInstruction(state, *opcode); break; }

		// "ADC A L" B:1 C:4 FLAGS: Z 0 H C
		case 0x8D: { unimplementedInstruction(state, *opcode); break; }

		// "ADC A [HL]" B:1 C:8 FLAGS: Z 0 H C
		case 0x8E: { unimplementedInstruction(state, *opcode); break; }

		// "ADC A A" B:1 C:4 FLAGS: Z 0 H C
		case 0x8F: { unimplementedInstruction(state, *opcode); break; }

		// "SUB A B" B:1 C:4 FLAGS: Z 1 H C
		case 0x90: { unimplementedInstruction(state, *opcode); break; }

		// "SUB A C" B:1 C:4 FLAGS: Z 1 H C
		case 0x91: { unimplementedInstruction(state, *opcode); break; }

		// "SUB A D" B:1 C:4 FLAGS: Z 1 H C
		case 0x92: { unimplementedInstruction(state, *opcode); break; }

		// "SUB A E" B:1 C:4 FLAGS: Z 1 H C
		case 0x93: { unimplementedInstruction(state, *opcode); break; }

		// "SUB A H" B:1 C:4 FLAGS: Z 1 H C
		case 0x94: { unimplementedInstruction(state, *opcode); break; }

		// "SUB A L" B:1 C:4 FLAGS: Z 1 H C
		case 0x95: { unimplementedInstruction(state, *opcode); break; }

		// "SUB A [HL]" B:1 C:8 FLAGS: Z 1 H C
		case 0x96: { unimplementedInstruction(state, *opcode); break; }

		// "SUB A A" B:1 C:4 FLAGS: 1 1 0 0
		case 0x97: { unimplementedInstruction(state, *opcode); break; }

		// "SBC A B" B:1 C:4 FLAGS: Z 1 H C
		case 0x98: { unimplementedInstruction(state, *opcode); break; }

		// "SBC A C" B:1 C:4 FLAGS: Z 1 H C
		case 0x99: { unimplementedInstruction(state, *opcode); break; }

		// "SBC A D" B:1 C:4 FLAGS: Z 1 H C
		case 0x9A: { unimplementedInstruction(state, *opcode); break; }

		// "SBC A E" B:1 C:4 FLAGS: Z 1 H C
		case 0x9B: { unimplementedInstruction(state, *opcode); break; }

		// "SBC A H" B:1 C:4 FLAGS: Z 1 H C
		case 0x9C: { unimplementedInstruction(state, *opcode); break; }

		// "SBC A L" B:1 C:4 FLAGS: Z 1 H C
		case 0x9D: { unimplementedInstruction(state, *opcode); break; }

		// "SBC A [HL]" B:1 C:8 FLAGS: Z 1 H C
		case 0x9E: { unimplementedInstruction(state, *opcode); break; }

		// "SBC A A" B:1 C:4 FLAGS: Z 1 H -
		case 0x9F: { unimplementedInstruction(state, *opcode); break; }

		// "AND A B" B:1 C:4 FLAGS: Z 0 1 0
		case 0xA0: { unimplementedInstruction(state, *opcode); break; }

		// "AND A C" B:1 C:4 FLAGS: Z 0 1 0
		case 0xA1: { unimplementedInstruction(state, *opcode); break; }

		// "AND A D" B:1 C:4 FLAGS: Z 0 1 0
		case 0xA2: { unimplementedInstruction(state, *opcode); break; }

		// "AND A E" B:1 C:4 FLAGS: Z 0 1 0
		case 0xA3: { unimplementedInstruction(state, *opcode); break; }

		// "AND A H" B:1 C:4 FLAGS: Z 0 1 0
		case 0xA4: { unimplementedInstruction(state, *opcode); break; }

		// "AND A L" B:1 C:4 FLAGS: Z 0 1 0
		case 0xA5: { unimplementedInstruction(state, *opcode); break; }

		// "AND A [HL]" B:1 C:8 FLAGS: Z 0 1 0
		case 0xA6: { unimplementedInstruction(state, *opcode); break; }

		// "AND A A" B:1 C:4 FLAGS: Z 0 1 0
		case 0xA7: { unimplementedInstruction(state, *opcode); break; }

		// "XOR A B" B:1 C:4 FLAGS: Z 0 0 0
		case 0xA8: { unimplementedInstruction(state, *opcode); break; }

		// "XOR A C" B:1 C:4 FLAGS: Z 0 0 0
		case 0xA9: { unimplementedInstruction(state, *opcode); break; }

		// "XOR A D" B:1 C:4 FLAGS: Z 0 0 0
		case 0xAA: { unimplementedInstruction(state, *opcode); break; }

		// "XOR A E" B:1 C:4 FLAGS: Z 0 0 0
		case 0xAB: { unimplementedInstruction(state, *opcode); break; }

		// "XOR A H" B:1 C:4 FLAGS: Z 0 0 0
		case 0xAC: { unimplementedInstruction(state, *opcode); break; }

		// "XOR A L" B:1 C:4 FLAGS: Z 0 0 0
		case 0xAD: { unimplementedInstruction(state, *opcode); break; }

		// "XOR A [HL]" B:1 C:8 FLAGS: Z 0 0 0
		case 0xAE: { unimplementedInstruction(state, *opcode); break; }

		// "XOR A A" B:1 C:4 FLAGS: 1 0 0 0
		case 0xAF:
		{
			state.a = state.a ^ state.a;
			state.flags.z = 1;
			state.flags.c = 0;
			state.flags.h = 0;
			state.flags.n = 0;
			break;
		}

		// "OR A B" B:1 C:4 FLAGS: Z 0 0 0
		case 0xB0: { unimplementedInstruction(state, *opcode); break; }

		// "OR A C" B:1 C:4 FLAGS: Z 0 0 0
		case 0xB1: { unimplementedInstruction(state, *opcode); break; }

		// "OR A D" B:1 C:4 FLAGS: Z 0 0 0
		case 0xB2: { unimplementedInstruction(state, *opcode); break; }

		// "OR A E" B:1 C:4 FLAGS: Z 0 0 0
		case 0xB3: { unimplementedInstruction(state, *opcode); break; }

		// "OR A H" B:1 C:4 FLAGS: Z 0 0 0
		case 0xB4: { unimplementedInstruction(state, *opcode); break; }

		// "OR A L" B:1 C:4 FLAGS: Z 0 0 0
		case 0xB5: { unimplementedInstruction(state, *opcode); break; }

		// "OR A [HL]" B:1 C:8 FLAGS: Z 0 0 0
		case 0xB6: { unimplementedInstruction(state, *opcode); break; }

		// "OR A A" B:1 C:4 FLAGS: Z 0 0 0
		case 0xB7: { unimplementedInstruction(state, *opcode); break; }

		// "CP A B" B:1 C:4 FLAGS: Z 1 H C
		case 0xB8: { unimplementedInstruction(state, *opcode); break; }

		// "CP A C" B:1 C:4 FLAGS: Z 1 H C
		case 0xB9: { unimplementedInstruction(state, *opcode); break; }

		// "CP A D" B:1 C:4 FLAGS: Z 1 H C
		case 0xBA: { unimplementedInstruction(state, *opcode); break; }

		// "CP A E" B:1 C:4 FLAGS: Z 1 H C
		case 0xBB: { unimplementedInstruction(state, *opcode); break; }

		// "CP A H" B:1 C:4 FLAGS: Z 1 H C
		case 0xBC: { unimplementedInstruction(state, *opcode); break; }

		// "CP A L" B:1 C:4 FLAGS: Z 1 H C
		case 0xBD: { unimplementedInstruction(state, *opcode); break; }

		// "CP A [HL]" B:1 C:8 FLAGS: Z 1 H C
		case 0xBE: { unimplementedInstruction(state, *opcode); break; }

		// "CP A A" B:1 C:4 FLAGS: 1 1 0 0
		case 0xBF: { unimplementedInstruction(state, *opcode); break; }

		// "ADD A n8" B:2 C:8 FLAGS: Z 0 H C
		case 0xC6: { unimplementedInstruction(state, *opcode); break; }

		// "ADC A n8" B:2 C:8 FLAGS: Z 0 H C
		case 0xCE: { unimplementedInstruction(state, *opcode); break; }

		// "SUB A n8" B:2 C:8 FLAGS: Z 1 H C
		case 0xD6: { unimplementedInstruction(state, *opcode); break; }

		// "SBC A n8" B:2 C:8 FLAGS: Z 1 H C
		case 0xDE: { unimplementedInstruction(state, *opcode); break; }

		// "AND A n8" B:2 C:8 FLAGS: Z 0 1 0
		case 0xE6: { unimplementedInstruction(state, *opcode); break; }

		// "XOR A n8" B:2 C:8 FLAGS: Z 0 0 0
		case 0xEE: { unimplementedInstruction(state, *opcode); break; }

		// "OR A n8" B:2 C:8 FLAGS: Z 0 0 0
		case 0xF6: { unimplementedInstruction(state, *opcode); break; }

		// "CP A n8" B:2 C:8 FLAGS: Z 1 H C
		case 0xFE:
		{
			if (state.a == opcode[1])
			{
				state.flags.z = 1;
			}
			state.pc++;
			break;
		}

		/********************************************************************************************
			16-bit Arithmetic/Logical Instructions
		*********************************************************************************************/

		// "INC BC" B:1 C:8 FLAGS: - - - -
		case 0x03:
		{
			state.c = state.c + 1;
			if (state.c == 0)
				state.b = state.b + 1;
			break;
		}

		// "ADD HL BC" B:1 C:8 FLAGS: - 0 H C
		case 0x09: { unimplementedInstruction(state, *opcode); break; }

		// "DEC BC" B:1 C:8 FLAGS: - - - -
		case 0x0B: { unimplementedInstruction(state, *opcode); break; }

		// "INC DE" B:1 C:8 FLAGS: - - - -
		case 0x13: { unimplementedInstruction(state, *opcode); break; }

		// "ADD HL DE" B:1 C:8 FLAGS: - 0 H C
		case 0x19: { unimplementedInstruction(state, *opcode); break; }

		// "DEC DE" B:1 C:8 FLAGS: - - - -
		case 0x1B: { unimplementedInstruction(state, *opcode); break; }

		// "INC HL" B:1 C:8 FLAGS: - - - -
		case 0x23: { unimplementedInstruction(state, *opcode); break; }

		// "ADD HL HL" B:1 C:8 FLAGS: - 0 H C
		case 0x29: { unimplementedInstruction(state, *opcode); break; }

		// "DEC HL" B:1 C:8 FLAGS: - - - -
		case 0x2B: { unimplementedInstruction(state, *opcode); break; }

		// "INC SP" B:1 C:8 FLAGS: - - - -
		case 0x33: { unimplementedInstruction(state, *opcode); break; }

		// "ADD HL SP" B:1 C:8 FLAGS: - 0 H C
		case 0x39: { unimplementedInstruction(state, *opcode); break; }

		// "DEC SP" B:1 C:8 FLAGS: - - - -
		case 0x3B: { unimplementedInstruction(state, *opcode); break; }

		// "ADD SP e8" B:2 C:16 FLAGS: 0 0 H C
		case 0xE8: { unimplementedInstruction(state, *opcode); break; }

		/********************************************************************************************
			8-bit Shift. Rotate and Bit Instructions
		*********************************************************************************************/

		// "RLCA" B:1 C:4 FLAGS: 0 0 0 C
		case 0x07: { unimplementedInstruction(state, *opcode); break; }

		// "RRCA" B:1 C:4 FLAGS: 0 0 0 C
		case 0x0F:
		{
			unimplementedInstruction(state, *opcode);
			break;
		}
		
		// "RLA" B:1 C:4 FLAGS: 0 0 0 C
		case 0x17: { unimplementedInstruction(state, *opcode); break; }

		// "RRA" B:1 C:4 FLAGS: 0 0 0 C
		case 0x1F: { unimplementedInstruction(state, *opcode); break; }

		
		default: 
			unimplementedInstruction(state, *opcode); 
			break;
	}
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

int Cpu::Disassemble(uint8_t *opcode, int pc)
{
	int opBytes = 1;

	switch (*opcode)
	{
		case 0x00: outputDisassembledInstruction("NOP", pc, opcode, 1); break; // "NOP" B:1 C:4 FLAGS: - - - -
		case 0x01: outputDisassembledInstruction("LD BC n16", pc, opcode, 3); opBytes = 3; break; // "LD BC n16" B:3 C:12 FLAGS: - - - -
		case 0x02: outputDisassembledInstruction("LD [BC] A", pc, opcode, 1); break; // "LD [BC] A" B:1 C:8 FLAGS: - - - -
		case 0x03: outputDisassembledInstruction("INC BC", pc, opcode, 1); break; // "INC BC" B:1 C:8 FLAGS: - - - -
		case 0x04: outputDisassembledInstruction("INC B", pc, opcode, 1); break; // "INC B" B:1 C:4 FLAGS: Z 0 H -
		case 0x05: outputDisassembledInstruction("DEC B", pc, opcode, 1); break; // "DEC B" B:1 C:4 FLAGS: Z 1 H -
		case 0x06: outputDisassembledInstruction("LD B n8", pc, opcode, 2); opBytes = 2; break; // "LD B n8" B:2 C:8 FLAGS: - - - -
		case 0x07: outputDisassembledInstruction("RLCA", pc, opcode, 1); break; // "RLCA" B:1 C:4 FLAGS: 0 0 0 C
		case 0x08: outputDisassembledInstruction("LD [a16] SP", pc, opcode, 3); opBytes = 3; break; // "LD [a16] SP" B:3 C:20 FLAGS: - - - -
		case 0x09: outputDisassembledInstruction("ADD HL BC", pc, opcode, 1); break; // "ADD HL BC" B:1 C:8 FLAGS: - 0 H C
		case 0x0A: outputDisassembledInstruction("LD A [BC]", pc, opcode, 1); break; // "LD A [BC]" B:1 C:8 FLAGS: - - - -
		case 0x0B: outputDisassembledInstruction("DEC BC", pc, opcode, 1); break; // "DEC BC" B:1 C:8 FLAGS: - - - -
		case 0x0C: outputDisassembledInstruction("INC C", pc, opcode, 1); break; // "INC C" B:1 C:4 FLAGS: Z 0 H -
		case 0x0D: outputDisassembledInstruction("DEC C", pc, opcode, 1); break; // "DEC C" B:1 C:4 FLAGS: Z 1 H -
		case 0x0E: outputDisassembledInstruction("LD C n8", pc, opcode, 2); opBytes = 2; break; // "LD C n8" B:2 C:8 FLAGS: - - - -
		case 0x0F: outputDisassembledInstruction("RRCA", pc, opcode, 1); break; // "RRCA" B:1 C:4 FLAGS: 0 0 0 C
		
		case 0x10: outputDisassembledInstruction("STOP n8", pc, opcode, 2); opBytes = 2; break; // "STOP n8" B:2 C:4 FLAGS: - - - -
		case 0x11: outputDisassembledInstruction("LD DE n16", pc, opcode, 3); opBytes = 3; break; // "LD DE n16" B:3 C:12 FLAGS: - - - -
		case 0x12: outputDisassembledInstruction("LD [DE] A", pc, opcode, 1); break; // "LD [DE] A" B:1 C:8 FLAGS: - - - -
		case 0x13: outputDisassembledInstruction("INC DE", pc, opcode, 1); break; // "INC DE" B:1 C:8 FLAGS: - - - -
		case 0x14: outputDisassembledInstruction("INC D", pc, opcode, 1); break; // "INC D" B:1 C:4 FLAGS: Z 0 H -
		case 0x15: outputDisassembledInstruction("DEC D", pc, opcode, 1); break; // "DEC D" B:1 C:4 FLAGS: Z 1 H -
		case 0x16: outputDisassembledInstruction("LD D n8", pc, opcode, 2); opBytes = 2; break; // "LD D n8" B:2 C:8 FLAGS: - - - -
		case 0x17: outputDisassembledInstruction("RLA", pc, opcode, 1); break; // "RLA" B:1 C:4 FLAGS: 0 0 0 C
		case 0x18: outputDisassembledInstruction("JR e8", pc, opcode, 2); opBytes = 2; break; // "JR e8" B:2 C:12 FLAGS: - - - -
		case 0x19: outputDisassembledInstruction("ADD HL DE", pc, opcode, 1); break; // "ADD HL DE" B:1 C:8 FLAGS: - 0 H C
		case 0x1A: outputDisassembledInstruction("LD A [DE]", pc, opcode, 1); break; // "LD A [DE]" B:1 C:8 FLAGS: - - - -
		case 0x1B: outputDisassembledInstruction("DEC DE", pc, opcode, 1); break; // "DEC DE" B:1 C:8 FLAGS: - - - -
		case 0x1C: outputDisassembledInstruction("INC E", pc, opcode, 1); break; // "INC E" B:1 C:4 FLAGS: Z 0 H -
		case 0x1D: outputDisassembledInstruction("DEC E", pc, opcode, 1); break; // "DEC E" B:1 C:4 FLAGS: Z 1 H -
		case 0x1E: outputDisassembledInstruction("LD E n8", pc, opcode, 2); opBytes = 2; break; // "LD E n8" B:2 C:8 FLAGS: - - - -
		case 0x1F: outputDisassembledInstruction("RRA", pc, opcode, 1); break; // "RRA" B:1 C:4 FLAGS: 0 0 0 C
		
		case 0x20: outputDisassembledInstruction("JR NZ e8", pc, opcode, 2); opBytes = 2; break; // "JR NZ e8" B:2 C:128 FLAGS: - - - -
		case 0x21: outputDisassembledInstruction("LD HL n16", pc, opcode, 3); opBytes = 3; break; // "LD HL n16" B:3 C:12 FLAGS: - - - -
		case 0x22: outputDisassembledInstruction("LD [HL] A", pc, opcode, 1); break; // "LD [HL] A" B:1 C:8 FLAGS: - - - -
		case 0x23: outputDisassembledInstruction("INC HL", pc, opcode, 1); break; // "INC HL" B:1 C:8 FLAGS: - - - -
		case 0x24: outputDisassembledInstruction("INC H", pc, opcode, 1); break; // "INC H" B:1 C:4 FLAGS: Z 0 H -
		case 0x25: outputDisassembledInstruction("DEC H", pc, opcode, 1); break; // "DEC H" B:1 C:4 FLAGS: Z 1 H -
		case 0x26: outputDisassembledInstruction("LD H n8", pc, opcode, 2); opBytes = 2; break; // "LD H n8" B:2 C:8 FLAGS: - - - -
		case 0x27: outputDisassembledInstruction("DAA", pc, opcode, 1); break; // "DAA" B:1 C:4 FLAGS: Z - 0 C
		case 0x28: outputDisassembledInstruction("JR Z e8", pc, opcode, 2); opBytes = 2; break; // "JR Z e8" B:2 C:128 FLAGS: - - - -
		case 0x29: outputDisassembledInstruction("ADD HL HL", pc, opcode, 1); break; // "ADD HL HL" B:1 C:8 FLAGS: - 0 H C
		case 0x2A: outputDisassembledInstruction("LD A [HL]", pc, opcode, 1); break; // "LD A [HL]" B:1 C:8 FLAGS: - - - -
		case 0x2B: outputDisassembledInstruction("DEC HL", pc, opcode, 1); break; // "DEC HL" B:1 C:8 FLAGS: - - - -
		case 0x2C: outputDisassembledInstruction("INC L", pc, opcode, 1); break; // "INC L" B:1 C:4 FLAGS: Z 0 H -
		case 0x2D: outputDisassembledInstruction("DEC L", pc, opcode, 1); break; // "DEC L" B:1 C:4 FLAGS: Z 1 H -
		case 0x2E: outputDisassembledInstruction("LD L n8", pc, opcode, 2); opBytes = 2; break; // "LD L n8" B:2 C:8 FLAGS: - - - -
		case 0x2F: outputDisassembledInstruction("CPL", pc, opcode, 1); break; // "CPL" B:1 C:4 FLAGS: - 1 1 -
		
		case 0x30: outputDisassembledInstruction("JR NC e8", pc, opcode, 2); opBytes = 2; break; // "JR NC e8" B:2 C:128 FLAGS: - - - -
		case 0x31: outputDisassembledInstruction("LD SP n16", pc, opcode, 3); opBytes = 3; break; // "LD SP n16" B:3 C:12 FLAGS: - - - -
		case 0x32: outputDisassembledInstruction("LD [HL] A", pc, opcode, 1); break; // "LD [HL] A" B:1 C:8 FLAGS: - - - -
		case 0x33: outputDisassembledInstruction("INC SP", pc, opcode, 1); break; // "INC SP" B:1 C:8 FLAGS: - - - -
		case 0x34: outputDisassembledInstruction("INC [HL]", pc, opcode, 1); break; // "INC [HL]" B:1 C:12 FLAGS: Z 0 H -
		case 0x35: outputDisassembledInstruction("DEC [HL]", pc, opcode, 1); break; // "DEC [HL]" B:1 C:12 FLAGS: Z 1 H -
		case 0x36: outputDisassembledInstruction("LD [HL] n8", pc, opcode, 2); opBytes = 2; break; // "LD [HL] n8" B:2 C:12 FLAGS: - - - -
		case 0x37: outputDisassembledInstruction("SCF", pc, opcode, 1); break; // "SCF" B:1 C:4 FLAGS: - 0 0 1
		case 0x38: outputDisassembledInstruction("JR C e8", pc, opcode, 2); opBytes = 2; break; // "JR C e8" B:2 C:128 FLAGS: - - - -
		case 0x39: outputDisassembledInstruction("ADD HL SP", pc, opcode, 1); break; // "ADD HL SP" B:1 C:8 FLAGS: - 0 H C
		case 0x3A: outputDisassembledInstruction("LD A [HL]", pc, opcode, 1); break; // "LD A [HL]" B:1 C:8 FLAGS: - - - -
		case 0x3B: outputDisassembledInstruction("DEC SP", pc, opcode, 1); break; // "DEC SP" B:1 C:8 FLAGS: - - - -
		case 0x3C: outputDisassembledInstruction("INC A", pc, opcode, 1); break; // "INC A" B:1 C:4 FLAGS: Z 0 H -
		case 0x3D: outputDisassembledInstruction("DEC A", pc, opcode, 1); break; // "DEC A" B:1 C:4 FLAGS: Z 1 H -
		case 0x3E: outputDisassembledInstruction("LD A n8", pc, opcode, 2); opBytes = 2; break; // "LD A n8" B:2 C:8 FLAGS: - - - -
		case 0x3F: outputDisassembledInstruction("CCF", pc, opcode, 1); break; // "CCF" B:1 C:4 FLAGS: - 0 0 C
		
		case 0x40: outputDisassembledInstruction("LD B B", pc, opcode, 1); break; // "LD B B" B:1 C:4 FLAGS: - - - -
		case 0x41: outputDisassembledInstruction("LD B C", pc, opcode, 1); break; // "LD B C" B:1 C:4 FLAGS: - - - -
		case 0x42: outputDisassembledInstruction("LD B D", pc, opcode, 1); break; // "LD B D" B:1 C:4 FLAGS: - - - -
		case 0x43: outputDisassembledInstruction("LD B E", pc, opcode, 1); break; // "LD B E" B:1 C:4 FLAGS: - - - -
		case 0x44: outputDisassembledInstruction("LD B H", pc, opcode, 1); break; // "LD B H" B:1 C:4 FLAGS: - - - -
		case 0x45: outputDisassembledInstruction("LD B L", pc, opcode, 1); break; // "LD B L" B:1 C:4 FLAGS: - - - -
		case 0x46: outputDisassembledInstruction("LD B [HL]", pc, opcode, 1); break; // "LD B [HL]" B:1 C:8 FLAGS: - - - -
		case 0x47: outputDisassembledInstruction("LD B A", pc, opcode, 1); break; // "LD B A" B:1 C:4 FLAGS: - - - -
		case 0x48: outputDisassembledInstruction("LD C B", pc, opcode, 1); break; // "LD C B" B:1 C:4 FLAGS: - - - -
		case 0x49: outputDisassembledInstruction("LD C C", pc, opcode, 1); break; // "LD C C" B:1 C:4 FLAGS: - - - -
		case 0x4A: outputDisassembledInstruction("LD C D", pc, opcode, 1); break; // "LD C D" B:1 C:4 FLAGS: - - - -
		case 0x4B: outputDisassembledInstruction("LD C E", pc, opcode, 1); break; // "LD C E" B:1 C:4 FLAGS: - - - -
		case 0x4C: outputDisassembledInstruction("LD C H", pc, opcode, 1); break; // "LD C H" B:1 C:4 FLAGS: - - - -
		case 0x4D: outputDisassembledInstruction("LD C L", pc, opcode, 1); break; // "LD C L" B:1 C:4 FLAGS: - - - -
		case 0x4E: outputDisassembledInstruction("LD C [HL]", pc, opcode, 1); break; // "LD C [HL]" B:1 C:8 FLAGS: - - - -
		case 0x4F: outputDisassembledInstruction("LD C A", pc, opcode, 1); break; // "LD C A" B:1 C:4 FLAGS: - - - -
		
		case 0x50: outputDisassembledInstruction("LD D B", pc, opcode, 1); break; // "LD D B" B:1 C:4 FLAGS: - - - -
		case 0x51: outputDisassembledInstruction("LD D C", pc, opcode, 1); break; // "LD D C" B:1 C:4 FLAGS: - - - -
		case 0x52: outputDisassembledInstruction("LD D D", pc, opcode, 1); break; // "LD D D" B:1 C:4 FLAGS: - - - -
		case 0x53: outputDisassembledInstruction("LD D E", pc, opcode, 1); break; // "LD D E" B:1 C:4 FLAGS: - - - -
		case 0x54: outputDisassembledInstruction("LD D H", pc, opcode, 1); break; // "LD D H" B:1 C:4 FLAGS: - - - -
		case 0x55: outputDisassembledInstruction("LD D L", pc, opcode, 1); break; // "LD D L" B:1 C:4 FLAGS: - - - -
		case 0x56: outputDisassembledInstruction("LD D [HL]", pc, opcode, 1); break; // "LD D [HL]" B:1 C:8 FLAGS: - - - -
		case 0x57: outputDisassembledInstruction("LD D A", pc, opcode, 1); break; // "LD D A" B:1 C:4 FLAGS: - - - -
		case 0x58: outputDisassembledInstruction("LD E B", pc, opcode, 1); break; // "LD E B" B:1 C:4 FLAGS: - - - -
		case 0x59: outputDisassembledInstruction("LD E C", pc, opcode, 1); break; // "LD E C" B:1 C:4 FLAGS: - - - -
		case 0x5A: outputDisassembledInstruction("LD E D", pc, opcode, 1); break; // "LD E D" B:1 C:4 FLAGS: - - - -
		case 0x5B: outputDisassembledInstruction("LD E E", pc, opcode, 1); break; // "LD E E" B:1 C:4 FLAGS: - - - -
		case 0x5C: outputDisassembledInstruction("LD E H", pc, opcode, 1); break; // "LD E H" B:1 C:4 FLAGS: - - - -
		case 0x5D: outputDisassembledInstruction("LD E L", pc, opcode, 1); break; // "LD E L" B:1 C:4 FLAGS: - - - -
		case 0x5E: outputDisassembledInstruction("LD E [HL]", pc, opcode, 1); break; // "LD E [HL]" B:1 C:8 FLAGS: - - - -
		case 0x5F: outputDisassembledInstruction("LD E A", pc, opcode, 1); break; // "LD E A" B:1 C:4 FLAGS: - - - -
		
		case 0x60: outputDisassembledInstruction("LD H B", pc, opcode, 1); break; // "LD H B" B:1 C:4 FLAGS: - - - -
		case 0x61: outputDisassembledInstruction("LD H C", pc, opcode, 1); break; // "LD H C" B:1 C:4 FLAGS: - - - -
		case 0x62: outputDisassembledInstruction("LD H D", pc, opcode, 1); break; // "LD H D" B:1 C:4 FLAGS: - - - -
		case 0x63: outputDisassembledInstruction("LD H E", pc, opcode, 1); break; // "LD H E" B:1 C:4 FLAGS: - - - -
		case 0x64: outputDisassembledInstruction("LD H H", pc, opcode, 1); break; // "LD H H" B:1 C:4 FLAGS: - - - -
		case 0x65: outputDisassembledInstruction("LD H L", pc, opcode, 1); break; // "LD H L" B:1 C:4 FLAGS: - - - -
		case 0x66: outputDisassembledInstruction("LD H [HL]", pc, opcode, 1); break; // "LD H [HL]" B:1 C:8 FLAGS: - - - -
		case 0x67: outputDisassembledInstruction("LD H A", pc, opcode, 1); break; // "LD H A" B:1 C:4 FLAGS: - - - -
		case 0x68: outputDisassembledInstruction("LD L B", pc, opcode, 1); break; // "LD L B" B:1 C:4 FLAGS: - - - -
		case 0x69: outputDisassembledInstruction("LD L C", pc, opcode, 1); break; // "LD L C" B:1 C:4 FLAGS: - - - -
		case 0x6A: outputDisassembledInstruction("LD L D", pc, opcode, 1); break; // "LD L D" B:1 C:4 FLAGS: - - - -
		case 0x6B: outputDisassembledInstruction("LD L E", pc, opcode, 1); break; // "LD L E" B:1 C:4 FLAGS: - - - -
		case 0x6C: outputDisassembledInstruction("LD L H", pc, opcode, 1); break; // "LD L H" B:1 C:4 FLAGS: - - - -
		case 0x6D: outputDisassembledInstruction("LD L L", pc, opcode, 1); break; // "LD L L" B:1 C:4 FLAGS: - - - -
		case 0x6E: outputDisassembledInstruction("LD L [HL]", pc, opcode, 1); break; // "LD L [HL]" B:1 C:8 FLAGS: - - - -
		case 0x6F: outputDisassembledInstruction("LD L A", pc, opcode, 1); break; // "LD L A" B:1 C:4 FLAGS: - - - -
		
		case 0x70: outputDisassembledInstruction("LD [HL] B", pc, opcode, 1); break; // "LD [HL] B" B:1 C:8 FLAGS: - - - -
		case 0x71: outputDisassembledInstruction("LD [HL] C", pc, opcode, 1); break; // "LD [HL] C" B:1 C:8 FLAGS: - - - -
		case 0x72: outputDisassembledInstruction("LD [HL] D", pc, opcode, 1); break; // "LD [HL] D" B:1 C:8 FLAGS: - - - -
		case 0x73: outputDisassembledInstruction("LD [HL] E", pc, opcode, 1); break; // "LD [HL] E" B:1 C:8 FLAGS: - - - -
		case 0x74: outputDisassembledInstruction("LD [HL] H", pc, opcode, 1); break; // "LD [HL] H" B:1 C:8 FLAGS: - - - -
		case 0x75: outputDisassembledInstruction("LD [HL] L", pc, opcode, 1); break; // "LD [HL] L" B:1 C:8 FLAGS: - - - -
		case 0x76: outputDisassembledInstruction("HALT", pc, opcode, 1); break; // "HALT" B:1 C:4 FLAGS: - - - -
		case 0x77: outputDisassembledInstruction("LD [HL] A", pc, opcode, 1); break; // "LD [HL] A" B:1 C:8 FLAGS: - - - -
		case 0x78: outputDisassembledInstruction("LD A B", pc, opcode, 1); break; // "LD A B" B:1 C:4 FLAGS: - - - -
		case 0x79: outputDisassembledInstruction("LD A C", pc, opcode, 1); break; // "LD A C" B:1 C:4 FLAGS: - - - -
		case 0x7A: outputDisassembledInstruction("LD A D", pc, opcode, 1); break; // "LD A D" B:1 C:4 FLAGS: - - - -
		case 0x7B: outputDisassembledInstruction("LD A E", pc, opcode, 1); break; // "LD A E" B:1 C:4 FLAGS: - - - -
		case 0x7C: outputDisassembledInstruction("LD A H", pc, opcode, 1); break; // "LD A H" B:1 C:4 FLAGS: - - - -
		case 0x7D: outputDisassembledInstruction("LD A L", pc, opcode, 1); break; // "LD A L" B:1 C:4 FLAGS: - - - -
		case 0x7E: outputDisassembledInstruction("LD A [HL]", pc, opcode, 1); break; // "LD A [HL]" B:1 C:8 FLAGS: - - - -
		case 0x7F: outputDisassembledInstruction("LD A A", pc, opcode, 1); break; // "LD A A" B:1 C:4 FLAGS: - - - -
		
		case 0x80: outputDisassembledInstruction("ADD A B", pc, opcode, 1); break; // "ADD A B" B:1 C:4 FLAGS: Z 0 H C
		case 0x81: outputDisassembledInstruction("ADD A C", pc, opcode, 1); break; // "ADD A C" B:1 C:4 FLAGS: Z 0 H C
		case 0x82: outputDisassembledInstruction("ADD A D", pc, opcode, 1); break; // "ADD A D" B:1 C:4 FLAGS: Z 0 H C
		case 0x83: outputDisassembledInstruction("ADD A E", pc, opcode, 1); break; // "ADD A E" B:1 C:4 FLAGS: Z 0 H C
		case 0x84: outputDisassembledInstruction("ADD A H", pc, opcode, 1); break; // "ADD A H" B:1 C:4 FLAGS: Z 0 H C
		case 0x85: outputDisassembledInstruction("ADD A L", pc, opcode, 1); break; // "ADD A L" B:1 C:4 FLAGS: Z 0 H C
		case 0x86: outputDisassembledInstruction("ADD A [HL]", pc, opcode, 1); break; // "ADD A [HL]" B:1 C:8 FLAGS: Z 0 H C
		case 0x87: outputDisassembledInstruction("ADD A A", pc, opcode, 1); break; // "ADD A A" B:1 C:4 FLAGS: Z 0 H C
		case 0x88: outputDisassembledInstruction("ADC A B", pc, opcode, 1); break; // "ADC A B" B:1 C:4 FLAGS: Z 0 H C
		case 0x89: outputDisassembledInstruction("ADC A C", pc, opcode, 1); break; // "ADC A C" B:1 C:4 FLAGS: Z 0 H C
		case 0x8A: outputDisassembledInstruction("ADC A D", pc, opcode, 1); break; // "ADC A D" B:1 C:4 FLAGS: Z 0 H C
		case 0x8B: outputDisassembledInstruction("ADC A E", pc, opcode, 1); break; // "ADC A E" B:1 C:4 FLAGS: Z 0 H C
		case 0x8C: outputDisassembledInstruction("ADC A H", pc, opcode, 1); break; // "ADC A H" B:1 C:4 FLAGS: Z 0 H C
		case 0x8D: outputDisassembledInstruction("ADC A L", pc, opcode, 1); break; // "ADC A L" B:1 C:4 FLAGS: Z 0 H C
		case 0x8E: outputDisassembledInstruction("ADC A [HL]", pc, opcode, 1); break; // "ADC A [HL]" B:1 C:8 FLAGS: Z 0 H C
		case 0x8F: outputDisassembledInstruction("ADC A A", pc, opcode, 1); break; // "ADC A A" B:1 C:4 FLAGS: Z 0 H C
		
		case 0x90: outputDisassembledInstruction("SUB A B", pc, opcode, 1); break; // "SUB A B" B:1 C:4 FLAGS: Z 1 H C
		case 0x91: outputDisassembledInstruction("SUB A C", pc, opcode, 1); break; // "SUB A C" B:1 C:4 FLAGS: Z 1 H C
		case 0x92: outputDisassembledInstruction("SUB A D", pc, opcode, 1); break; // "SUB A D" B:1 C:4 FLAGS: Z 1 H C
		case 0x93: outputDisassembledInstruction("SUB A E", pc, opcode, 1); break; // "SUB A E" B:1 C:4 FLAGS: Z 1 H C
		case 0x94: outputDisassembledInstruction("SUB A H", pc, opcode, 1); break; // "SUB A H" B:1 C:4 FLAGS: Z 1 H C
		case 0x95: outputDisassembledInstruction("SUB A L", pc, opcode, 1); break; // "SUB A L" B:1 C:4 FLAGS: Z 1 H C
		case 0x96: outputDisassembledInstruction("SUB A [HL]", pc, opcode, 1); break; // "SUB A [HL]" B:1 C:8 FLAGS: Z 1 H C
		case 0x97: outputDisassembledInstruction("SUB A A", pc, opcode, 1); break; // "SUB A A" B:1 C:4 FLAGS: 1 1 0 0
		case 0x98: outputDisassembledInstruction("SBC A B", pc, opcode, 1); break; // "SBC A B" B:1 C:4 FLAGS: Z 1 H C
		case 0x99: outputDisassembledInstruction("SBC A C", pc, opcode, 1); break; // "SBC A C" B:1 C:4 FLAGS: Z 1 H C
		case 0x9A: outputDisassembledInstruction("SBC A D", pc, opcode, 1); break; // "SBC A D" B:1 C:4 FLAGS: Z 1 H C
		case 0x9B: outputDisassembledInstruction("SBC A E", pc, opcode, 1); break; // "SBC A E" B:1 C:4 FLAGS: Z 1 H C
		case 0x9C: outputDisassembledInstruction("SBC A H", pc, opcode, 1); break; // "SBC A H" B:1 C:4 FLAGS: Z 1 H C
		case 0x9D: outputDisassembledInstruction("SBC A L", pc, opcode, 1); break; // "SBC A L" B:1 C:4 FLAGS: Z 1 H C
		case 0x9E: outputDisassembledInstruction("SBC A [HL]", pc, opcode, 1); break; // "SBC A [HL]" B:1 C:8 FLAGS: Z 1 H C
		case 0x9F: outputDisassembledInstruction("SBC A A", pc, opcode, 1); break; // "SBC A A" B:1 C:4 FLAGS: Z 1 H -
		
		case 0xA0: outputDisassembledInstruction("AND A B", pc, opcode, 1); break; // "AND A B" B:1 C:4 FLAGS: Z 0 1 0
		case 0xA1: outputDisassembledInstruction("AND A C", pc, opcode, 1); break; // "AND A C" B:1 C:4 FLAGS: Z 0 1 0
		case 0xA2: outputDisassembledInstruction("AND A D", pc, opcode, 1); break; // "AND A D" B:1 C:4 FLAGS: Z 0 1 0
		case 0xA3: outputDisassembledInstruction("AND A E", pc, opcode, 1); break; // "AND A E" B:1 C:4 FLAGS: Z 0 1 0
		case 0xA4: outputDisassembledInstruction("AND A H", pc, opcode, 1); break; // "AND A H" B:1 C:4 FLAGS: Z 0 1 0
		case 0xA5: outputDisassembledInstruction("AND A L", pc, opcode, 1); break; // "AND A L" B:1 C:4 FLAGS: Z 0 1 0
		case 0xA6: outputDisassembledInstruction("AND A [HL]", pc, opcode, 1); break; // "AND A [HL]" B:1 C:8 FLAGS: Z 0 1 0
		case 0xA7: outputDisassembledInstruction("AND A A", pc, opcode, 1); break; // "AND A A" B:1 C:4 FLAGS: Z 0 1 0
		case 0xA8: outputDisassembledInstruction("XOR A B", pc, opcode, 1); break; // "XOR A B" B:1 C:4 FLAGS: Z 0 0 0
		case 0xA9: outputDisassembledInstruction("XOR A C", pc, opcode, 1); break; // "XOR A C" B:1 C:4 FLAGS: Z 0 0 0
		case 0xAA: outputDisassembledInstruction("XOR A D", pc, opcode, 1); break; // "XOR A D" B:1 C:4 FLAGS: Z 0 0 0
		case 0xAB: outputDisassembledInstruction("XOR A E", pc, opcode, 1); break; // "XOR A E" B:1 C:4 FLAGS: Z 0 0 0
		case 0xAC: outputDisassembledInstruction("XOR A H", pc, opcode, 1); break; // "XOR A H" B:1 C:4 FLAGS: Z 0 0 0
		case 0xAD: outputDisassembledInstruction("XOR A L", pc, opcode, 1); break; // "XOR A L" B:1 C:4 FLAGS: Z 0 0 0
		case 0xAE: outputDisassembledInstruction("XOR A [HL]", pc, opcode, 1); break; // "XOR A [HL]" B:1 C:8 FLAGS: Z 0 0 0
		case 0xAF: outputDisassembledInstruction("XOR A A", pc, opcode, 1); break; // "XOR A A" B:1 C:4 FLAGS: 1 0 0 0
		
		case 0xB0: outputDisassembledInstruction("OR A B", pc, opcode, 1); break; // "OR A B" B:1 C:4 FLAGS: Z 0 0 0
		case 0xB1: outputDisassembledInstruction("OR A C", pc, opcode, 1); break; // "OR A C" B:1 C:4 FLAGS: Z 0 0 0
		case 0xB2: outputDisassembledInstruction("OR A D", pc, opcode, 1); break; // "OR A D" B:1 C:4 FLAGS: Z 0 0 0
		case 0xB3: outputDisassembledInstruction("OR A E", pc, opcode, 1); break; // "OR A E" B:1 C:4 FLAGS: Z 0 0 0
		case 0xB4: outputDisassembledInstruction("OR A H", pc, opcode, 1); break; // "OR A H" B:1 C:4 FLAGS: Z 0 0 0
		case 0xB5: outputDisassembledInstruction("OR A L", pc, opcode, 1); break; // "OR A L" B:1 C:4 FLAGS: Z 0 0 0
		case 0xB6: outputDisassembledInstruction("OR A [HL]", pc, opcode, 1); break; // "OR A [HL]" B:1 C:8 FLAGS: Z 0 0 0
		case 0xB7: outputDisassembledInstruction("OR A A", pc, opcode, 1); break; // "OR A A" B:1 C:4 FLAGS: Z 0 0 0
		case 0xB8: outputDisassembledInstruction("CP A B", pc, opcode, 1); break; // "CP A B" B:1 C:4 FLAGS: Z 1 H C
		case 0xB9: outputDisassembledInstruction("CP A C", pc, opcode, 1); break; // "CP A C" B:1 C:4 FLAGS: Z 1 H C
		case 0xBA: outputDisassembledInstruction("CP A D", pc, opcode, 1); break; // "CP A D" B:1 C:4 FLAGS: Z 1 H C
		case 0xBB: outputDisassembledInstruction("CP A E", pc, opcode, 1); break; // "CP A E" B:1 C:4 FLAGS: Z 1 H C
		case 0xBC: outputDisassembledInstruction("CP A H", pc, opcode, 1); break; // "CP A H" B:1 C:4 FLAGS: Z 1 H C
		case 0xBD: outputDisassembledInstruction("CP A L", pc, opcode, 1); break; // "CP A L" B:1 C:4 FLAGS: Z 1 H C
		case 0xBE: outputDisassembledInstruction("CP A [HL]", pc, opcode, 1); break; // "CP A [HL]" B:1 C:8 FLAGS: Z 1 H C
		case 0xBF: outputDisassembledInstruction("CP A A", pc, opcode, 1); break; // "CP A A" B:1 C:4 FLAGS: 1 1 0 0
		
		case 0xC0: outputDisassembledInstruction("RET NZ", pc, opcode, 1); break; // "RET NZ" B:1 C:208 FLAGS: - - - -
		case 0xC1: outputDisassembledInstruction("POP BC", pc, opcode, 1); break; // "POP BC" B:1 C:12 FLAGS: - - - -
		case 0xC2: outputDisassembledInstruction("JP NZ a16", pc, opcode, 3); opBytes = 3; break; // "JP NZ a16" B:3 C:1612 FLAGS: - - - -
		case 0xC3: outputDisassembledInstruction("JP a16", pc, opcode, 3); opBytes = 3; break; // "JP a16" B:3 C:16 FLAGS: - - - -
		case 0xC4: outputDisassembledInstruction("CALL NZ a16", pc, opcode, 3); opBytes = 3; break; // "CALL NZ a16" B:3 C:2412 FLAGS: - - - -
		case 0xC5: outputDisassembledInstruction("PUSH BC", pc, opcode, 1); break; // "PUSH BC" B:1 C:16 FLAGS: - - - -
		case 0xC6: outputDisassembledInstruction("ADD A n8", pc, opcode, 2); opBytes = 2; break; // "ADD A n8" B:2 C:8 FLAGS: Z 0 H C
		case 0xC7: outputDisassembledInstruction("RST $00", pc, opcode, 1); break; // "RST $00" B:1 C:16 FLAGS: - - - -
		case 0xC8: outputDisassembledInstruction("RET Z", pc, opcode, 1); break; // "RET Z" B:1 C:208 FLAGS: - - - -
		case 0xC9: outputDisassembledInstruction("RET", pc, opcode, 1); break; // "RET" B:1 C:16 FLAGS: - - - -
		case 0xCA: outputDisassembledInstruction("JP Z a16", pc, opcode, 3); opBytes = 3; break; // "JP Z a16" B:3 C:1612 FLAGS: - - - -
		
		// "PREFIX" B:1 C:4 FLAGS: - - - -
		// Address CB triggers a 16 bit opcode 
		case 0xCB:
		{
			//uint8_t firstChar = *opcode;
			//uint8_t secondChar = opcode[1];
			//uint16_t opcode16 = ((uint16_t)firstChar << 8) | secondChar;
			disasseble16bit(opcode, pc);
			opBytes = 2;
			break;
		}

		case 0xCC: outputDisassembledInstruction("CALL Z a16", pc, opcode, 3); opBytes = 3; break; // "CALL Z a16" B:3 C:2412 FLAGS: - - - -
		case 0xCD: outputDisassembledInstruction("CALL a16", pc, opcode, 3); opBytes = 3; break; // "CALL a16" B:3 C:24 FLAGS: - - - -
		case 0xCE: outputDisassembledInstruction("ADC A n8", pc, opcode, 2); opBytes = 2; break; // "ADC A n8" B:2 C:8 FLAGS: Z 0 H C
		case 0xCF: outputDisassembledInstruction("RST $08", pc, opcode, 1); break; // "RST $08" B:1 C:16 FLAGS: - - - -
		
		case 0xD0: outputDisassembledInstruction("RET NC", pc, opcode, 1); break; // "RET NC" B:1 C:208 FLAGS: - - - -
		case 0xD1: outputDisassembledInstruction("POP DE", pc, opcode, 1); break; // "POP DE" B:1 C:12 FLAGS: - - - -
		case 0xD2: outputDisassembledInstruction("JP NC a16", pc, opcode, 3); opBytes = 3; break; // "JP NC a16" B:3 C:1612 FLAGS: - - - -
		case 0xD3: outputDisassembledInstruction("ILLEGAL_D3", pc, opcode, 1); break; // "ILLEGAL_D3" B:1 C:4 FLAGS: - - - -
		case 0xD4: outputDisassembledInstruction("CALL NC a16", pc, opcode, 3); opBytes = 3; break; // "CALL NC a16" B:3 C:2412 FLAGS: - - - -
		case 0xD5: outputDisassembledInstruction("PUSH DE", pc, opcode, 1); break; // "PUSH DE" B:1 C:16 FLAGS: - - - -
		case 0xD6: outputDisassembledInstruction("SUB A n8", pc, opcode, 2); opBytes = 2; break; // "SUB A n8" B:2 C:8 FLAGS: Z 1 H C
		case 0xD7: outputDisassembledInstruction("RST $10", pc, opcode, 1); break; // "RST $10" B:1 C:16 FLAGS: - - - -
		case 0xD8: outputDisassembledInstruction("RET C", pc, opcode, 1); break; // "RET C" B:1 C:208 FLAGS: - - - -
		case 0xD9: outputDisassembledInstruction("RETI", pc, opcode, 1); break; // "RETI" B:1 C:16 FLAGS: - - - -
		case 0xDA: outputDisassembledInstruction("JP C a16", pc, opcode, 3); opBytes = 3; break; // "JP C a16" B:3 C:1612 FLAGS: - - - -
		case 0xDB: outputDisassembledInstruction("ILLEGAL_DB", pc, opcode, 1); break; // "ILLEGAL_DB" B:1 C:4 FLAGS: - - - -
		case 0xDC: outputDisassembledInstruction("CALL C a16", pc, opcode, 3); opBytes = 3; break; // "CALL C a16" B:3 C:2412 FLAGS: - - - -
		case 0xDD: outputDisassembledInstruction("ILLEGAL_DD", pc, opcode, 1); break; // "ILLEGAL_DD" B:1 C:4 FLAGS: - - - -
		case 0xDE: outputDisassembledInstruction("SBC A n8", pc, opcode, 2); opBytes = 2; break; // "SBC A n8" B:2 C:8 FLAGS: Z 1 H C
		case 0xDF: outputDisassembledInstruction("RST $18", pc, opcode, 1); break; // "RST $18" B:1 C:16 FLAGS: - - - -
		
		case 0xE0: outputDisassembledInstruction("LDH [a8] A", pc, opcode, 2); opBytes = 2; break; // "LDH [a8] A" B:2 C:12 FLAGS: - - - -
		case 0xE1: outputDisassembledInstruction("POP HL", pc, opcode, 1); break; // "POP HL" B:1 C:12 FLAGS: - - - -
		case 0xE2: outputDisassembledInstruction("LD [C] A", pc, opcode, 1); break; // "LD [C] A" B:1 C:8 FLAGS: - - - -
		case 0xE3: outputDisassembledInstruction("ILLEGAL_E3", pc, opcode, 1); break; // "ILLEGAL_E3" B:1 C:4 FLAGS: - - - -
		case 0xE4: outputDisassembledInstruction("ILLEGAL_E4", pc, opcode, 1); break; // "ILLEGAL_E4" B:1 C:4 FLAGS: - - - -
		case 0xE5: outputDisassembledInstruction("PUSH HL", pc, opcode, 1); break; // "PUSH HL" B:1 C:16 FLAGS: - - - -
		case 0xE6: outputDisassembledInstruction("AND A n8", pc, opcode, 2); opBytes = 2; break; // "AND A n8" B:2 C:8 FLAGS: Z 0 1 0
		case 0xE7: outputDisassembledInstruction("RST $20", pc, opcode, 1); break; // "RST $20" B:1 C:16 FLAGS: - - - -
		case 0xE8: outputDisassembledInstruction("ADD SP e8", pc, opcode, 2); opBytes = 2; break; // "ADD SP e8" B:2 C:16 FLAGS: 0 0 H C
		case 0xE9: outputDisassembledInstruction("JP HL", pc, opcode, 1); break; // "JP HL" B:1 C:4 FLAGS: - - - -
		case 0xEA: outputDisassembledInstruction("LD [a16] A", pc, opcode, 3); opBytes = 3; break; // "LD [a16] A" B:3 C:16 FLAGS: - - - -
		case 0xEB: outputDisassembledInstruction("ILLEGAL_EB", pc, opcode, 1); break; // "ILLEGAL_EB" B:1 C:4 FLAGS: - - - -
		case 0xEC: outputDisassembledInstruction("ILLEGAL_EC", pc, opcode, 1); break; // "ILLEGAL_EC" B:1 C:4 FLAGS: - - - -
		case 0xED: outputDisassembledInstruction("ILLEGAL_ED", pc, opcode, 1); break; // "ILLEGAL_ED" B:1 C:4 FLAGS: - - - -
		case 0xEE: outputDisassembledInstruction("XOR A n8", pc, opcode, 2); opBytes = 2; break; // "XOR A n8" B:2 C:8 FLAGS: Z 0 0 0
		case 0xEF: outputDisassembledInstruction("RST $28", pc, opcode, 1); break; // "RST $28" B:1 C:16 FLAGS: - - - -
		
		case 0xF0: outputDisassembledInstruction("LDH A [a8]", pc, opcode, 2); opBytes = 2; break; // "LDH A [a8]" B:2 C:12 FLAGS: - - - -
		case 0xF1: outputDisassembledInstruction("POP AF", pc, opcode, 1); break; // "POP AF" B:1 C:12 FLAGS: Z N H C
		case 0xF2: outputDisassembledInstruction("LD A [C]", pc, opcode, 1); break; // "LD A [C]" B:1 C:8 FLAGS: - - - -
		case 0xF3: outputDisassembledInstruction("DI", pc, opcode, 1); break; // "DI" B:1 C:4 FLAGS: - - - -
		case 0xF4: outputDisassembledInstruction("ILLEGAL_F4", pc, opcode, 1); break; // "ILLEGAL_F4" B:1 C:4 FLAGS: - - - -
		case 0xF5: outputDisassembledInstruction("PUSH AF", pc, opcode, 1); break; // "PUSH AF" B:1 C:16 FLAGS: - - - -
		case 0xF6: outputDisassembledInstruction("OR A n8", pc, opcode, 2); opBytes = 2; break; // "OR A n8" B:2 C:8 FLAGS: Z 0 0 0
		case 0xF7: outputDisassembledInstruction("RST $30", pc, opcode, 1); break; // "RST $30" B:1 C:16 FLAGS: - - - -
		case 0xF8: outputDisassembledInstruction("LD HL SP e8", pc, opcode, 2); opBytes = 2; break; // "LD HL SP e8" B:2 C:12 FLAGS: 0 0 H C
		case 0xF9: outputDisassembledInstruction("LD SP HL", pc, opcode, 1); break; // "LD SP HL" B:1 C:8 FLAGS: - - - -
		case 0xFA: outputDisassembledInstruction("LD A [a16]", pc, opcode, 3); opBytes = 3; break; // "LD A [a16]" B:3 C:16 FLAGS: - - - -
		case 0xFB: outputDisassembledInstruction("EI", pc, opcode, 1); break; // "EI" B:1 C:4 FLAGS: - - - -
		case 0xFC: outputDisassembledInstruction("ILLEGAL_FC", pc, opcode, 1); break; // "ILLEGAL_FC" B:1 C:4 FLAGS: - - - -
		case 0xFD: outputDisassembledInstruction("ILLEGAL_FD", pc, opcode, 1); break; // "ILLEGAL_FD" B:1 C:4 FLAGS: - - - -
		case 0xFE: outputDisassembledInstruction("CP A n8", pc, opcode, 2); opBytes = 2; break; // "CP A n8" B:2 C:8 FLAGS: Z 1 H C
		case 0xFF: outputDisassembledInstruction("RST $38", pc, opcode, 1); break; // "RST $38" B:1 C:16 FLAGS: - - - -

		default: outputDisassembledInstruction("", pc, opcode, 1); break;
	}

	return opBytes;
}

void disasseble16bit(uint8_t *opcode, int pc)
{
	// Note: All 16 bit opcodes appear to be 2 bytes.

	switch (*opcode)
	{
		case 0xCB00: outputDisassembledInstruction("RLC B", pc, opcode, 2); break; // "RLC B" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB01: outputDisassembledInstruction("RLC C", pc, opcode, 2); break; // "RLC C" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB02: outputDisassembledInstruction("RLC D", pc, opcode, 2); break; // "RLC D" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB03: outputDisassembledInstruction("RLC E", pc, opcode, 2); break; // "RLC E" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB04: outputDisassembledInstruction("RLC H", pc, opcode, 2); break; // "RLC H" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB05: outputDisassembledInstruction("RLC L", pc, opcode, 2); break; // "RLC L" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB06: outputDisassembledInstruction("RLC [HL]", pc, opcode, 2); break; // "RLC [HL]" B:2 C:16 FLAGS: Z 0 0 C
		case 0xCB07: outputDisassembledInstruction("RLC A", pc, opcode, 2); break; // "RLC A" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB08: outputDisassembledInstruction("RRC B", pc, opcode, 2); break; // "RRC B" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB09: outputDisassembledInstruction("RRC C", pc, opcode, 2); break; // "RRC C" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB0A: outputDisassembledInstruction("RRC D", pc, opcode, 2); break; // "RRC D" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB0B: outputDisassembledInstruction("RRC E", pc, opcode, 2); break; // "RRC E" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB0C: outputDisassembledInstruction("RRC H", pc, opcode, 2); break; // "RRC H" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB0D: outputDisassembledInstruction("RRC L", pc, opcode, 2); break; // "RRC L" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB0E: outputDisassembledInstruction("RRC [HL]", pc, opcode, 2); break; // "RRC [HL]" B:2 C:16 FLAGS: Z 0 0 C
		case 0xCB0F: outputDisassembledInstruction("RRC A", pc, opcode, 2); break; // "RRC A" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB10: outputDisassembledInstruction("RL B", pc, opcode, 2); break; // "RL B" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB11: outputDisassembledInstruction("RL C", pc, opcode, 2); break; // "RL C" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB12: outputDisassembledInstruction("RL D", pc, opcode, 2); break; // "RL D" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB13: outputDisassembledInstruction("RL E", pc, opcode, 2); break; // "RL E" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB14: outputDisassembledInstruction("RL H", pc, opcode, 2); break; // "RL H" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB15: outputDisassembledInstruction("RL L", pc, opcode, 2); break; // "RL L" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB16: outputDisassembledInstruction("RL [HL]", pc, opcode, 2); break; // "RL [HL]" B:2 C:16 FLAGS: Z 0 0 C
		case 0xCB17: outputDisassembledInstruction("RL A", pc, opcode, 2); break; // "RL A" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB18: outputDisassembledInstruction("RR B", pc, opcode, 2); break; // "RR B" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB19: outputDisassembledInstruction("RR C", pc, opcode, 2); break; // "RR C" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB1A: outputDisassembledInstruction("RR D", pc, opcode, 2); break; // "RR D" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB1B: outputDisassembledInstruction("RR E", pc, opcode, 2); break; // "RR E" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB1C: outputDisassembledInstruction("RR H", pc, opcode, 2); break; // "RR H" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB1D: outputDisassembledInstruction("RR L", pc, opcode, 2); break; // "RR L" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB1E: outputDisassembledInstruction("RR [HL]", pc, opcode, 2); break; // "RR [HL]" B:2 C:16 FLAGS: Z 0 0 C
		case 0xCB1F: outputDisassembledInstruction("RR A", pc, opcode, 2); break; // "RR A" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB20: outputDisassembledInstruction("SLA B", pc, opcode, 2); break; // "SLA B" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB21: outputDisassembledInstruction("SLA C", pc, opcode, 2); break; // "SLA C" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB22: outputDisassembledInstruction("SLA D", pc, opcode, 2); break; // "SLA D" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB23: outputDisassembledInstruction("SLA E", pc, opcode, 2); break; // "SLA E" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB24: outputDisassembledInstruction("SLA H", pc, opcode, 2); break; // "SLA H" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB25: outputDisassembledInstruction("SLA L", pc, opcode, 2); break; // "SLA L" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB26: outputDisassembledInstruction("SLA [HL]", pc, opcode, 2); break; // "SLA [HL]" B:2 C:16 FLAGS: Z 0 0 C
		case 0xCB27: outputDisassembledInstruction("SLA A", pc, opcode, 2); break; // "SLA A" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB28: outputDisassembledInstruction("SRA B", pc, opcode, 2); break; // "SRA B" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB29: outputDisassembledInstruction("SRA C", pc, opcode, 2); break; // "SRA C" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB2A: outputDisassembledInstruction("SRA D", pc, opcode, 2); break; // "SRA D" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB2B: outputDisassembledInstruction("SRA E", pc, opcode, 2); break; // "SRA E" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB2C: outputDisassembledInstruction("SRA H", pc, opcode, 2); break; // "SRA H" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB2D: outputDisassembledInstruction("SRA L", pc, opcode, 2); break; // "SRA L" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB2E: outputDisassembledInstruction("SRA [HL]", pc, opcode, 2); break; // "SRA [HL]" B:2 C:16 FLAGS: Z 0 0 C
		case 0xCB2F: outputDisassembledInstruction("SRA A", pc, opcode, 2); break; // "SRA A" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB30: outputDisassembledInstruction("SWAP B", pc, opcode, 2); break; // "SWAP B" B:2 C:8 FLAGS: Z 0 0 0
		case 0xCB31: outputDisassembledInstruction("SWAP C", pc, opcode, 2); break; // "SWAP C" B:2 C:8 FLAGS: Z 0 0 0
		case 0xCB32: outputDisassembledInstruction("SWAP D", pc, opcode, 2); break; // "SWAP D" B:2 C:8 FLAGS: Z 0 0 0
		case 0xCB33: outputDisassembledInstruction("SWAP E", pc, opcode, 2); break; // "SWAP E" B:2 C:8 FLAGS: Z 0 0 0
		case 0xCB34: outputDisassembledInstruction("SWAP H", pc, opcode, 2); break; // "SWAP H" B:2 C:8 FLAGS: Z 0 0 0
		case 0xCB35: outputDisassembledInstruction("SWAP L", pc, opcode, 2); break; // "SWAP L" B:2 C:8 FLAGS: Z 0 0 0
		case 0xCB36: outputDisassembledInstruction("SWAP [HL]", pc, opcode, 2); break; // "SWAP [HL]" B:2 C:16 FLAGS: Z 0 0 0
		case 0xCB37: outputDisassembledInstruction("SWAP A", pc, opcode, 2); break; // "SWAP A" B:2 C:8 FLAGS: Z 0 0 0
		case 0xCB38: outputDisassembledInstruction("SRL B", pc, opcode, 2); break; // "SRL B" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB39: outputDisassembledInstruction("SRL C", pc, opcode, 2); break; // "SRL C" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB3A: outputDisassembledInstruction("SRL D", pc, opcode, 2); break; // "SRL D" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB3B: outputDisassembledInstruction("SRL E", pc, opcode, 2); break; // "SRL E" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB3C: outputDisassembledInstruction("SRL H", pc, opcode, 2); break; // "SRL H" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB3D: outputDisassembledInstruction("SRL L", pc, opcode, 2); break; // "SRL L" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB3E: outputDisassembledInstruction("SRL [HL]", pc, opcode, 2); break; // "SRL [HL]" B:2 C:16 FLAGS: Z 0 0 C
		case 0xCB3F: outputDisassembledInstruction("SRL A", pc, opcode, 2); break; // "SRL A" B:2 C:8 FLAGS: Z 0 0 C
		case 0xCB40: outputDisassembledInstruction("BIT 0 B", pc, opcode, 2); break; // "BIT 0 B" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB41: outputDisassembledInstruction("BIT 0 C", pc, opcode, 2); break; // "BIT 0 C" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB42: outputDisassembledInstruction("BIT 0 D", pc, opcode, 2); break; // "BIT 0 D" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB43: outputDisassembledInstruction("BIT 0 E", pc, opcode, 2); break; // "BIT 0 E" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB44: outputDisassembledInstruction("BIT 0 H", pc, opcode, 2); break; // "BIT 0 H" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB45: outputDisassembledInstruction("BIT 0 L", pc, opcode, 2); break; // "BIT 0 L" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB46: outputDisassembledInstruction("BIT 0 [HL]", pc, opcode, 2); break; // "BIT 0 [HL]" B:2 C:12 FLAGS: Z 0 1 -
		case 0xCB47: outputDisassembledInstruction("BIT 0 A", pc, opcode, 2); break; // "BIT 0 A" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB48: outputDisassembledInstruction("BIT 1 B", pc, opcode, 2); break; // "BIT 1 B" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB49: outputDisassembledInstruction("BIT 1 C", pc, opcode, 2); break; // "BIT 1 C" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB4A: outputDisassembledInstruction("BIT 1 D", pc, opcode, 2); break; // "BIT 1 D" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB4B: outputDisassembledInstruction("BIT 1 E", pc, opcode, 2); break; // "BIT 1 E" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB4C: outputDisassembledInstruction("BIT 1 H", pc, opcode, 2); break; // "BIT 1 H" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB4D: outputDisassembledInstruction("BIT 1 L", pc, opcode, 2); break; // "BIT 1 L" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB4E: outputDisassembledInstruction("BIT 1 [HL]", pc, opcode, 2); break; // "BIT 1 [HL]" B:2 C:12 FLAGS: Z 0 1 -
		case 0xCB4F: outputDisassembledInstruction("BIT 1 A", pc, opcode, 2); break; // "BIT 1 A" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB50: outputDisassembledInstruction("BIT 2 B", pc, opcode, 2); break; // "BIT 2 B" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB51: outputDisassembledInstruction("BIT 2 C", pc, opcode, 2); break; // "BIT 2 C" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB52: outputDisassembledInstruction("BIT 2 D", pc, opcode, 2); break; // "BIT 2 D" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB53: outputDisassembledInstruction("BIT 2 E", pc, opcode, 2); break; // "BIT 2 E" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB54: outputDisassembledInstruction("BIT 2 H", pc, opcode, 2); break; // "BIT 2 H" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB55: outputDisassembledInstruction("BIT 2 L", pc, opcode, 2); break; // "BIT 2 L" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB56: outputDisassembledInstruction("BIT 2 [HL]", pc, opcode, 2); break; // "BIT 2 [HL]" B:2 C:12 FLAGS: Z 0 1 -
		case 0xCB57: outputDisassembledInstruction("BIT 2 A", pc, opcode, 2); break; // "BIT 2 A" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB58: outputDisassembledInstruction("BIT 3 B", pc, opcode, 2); break; // "BIT 3 B" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB59: outputDisassembledInstruction("BIT 3 C", pc, opcode, 2); break; // "BIT 3 C" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB5A: outputDisassembledInstruction("BIT 3 D", pc, opcode, 2); break; // "BIT 3 D" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB5B: outputDisassembledInstruction("BIT 3 E", pc, opcode, 2); break; // "BIT 3 E" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB5C: outputDisassembledInstruction("BIT 3 H", pc, opcode, 2); break; // "BIT 3 H" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB5D: outputDisassembledInstruction("BIT 3 L", pc, opcode, 2); break; // "BIT 3 L" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB5E: outputDisassembledInstruction("BIT 3 [HL]", pc, opcode, 2); break; // "BIT 3 [HL]" B:2 C:12 FLAGS: Z 0 1 -
		case 0xCB5F: outputDisassembledInstruction("BIT 3 A", pc, opcode, 2); break; // "BIT 3 A" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB60: outputDisassembledInstruction("BIT 4 B", pc, opcode, 2); break; // "BIT 4 B" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB61: outputDisassembledInstruction("BIT 4 C", pc, opcode, 2); break; // "BIT 4 C" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB62: outputDisassembledInstruction("BIT 4 D", pc, opcode, 2); break; // "BIT 4 D" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB63: outputDisassembledInstruction("BIT 4 E", pc, opcode, 2); break; // "BIT 4 E" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB64: outputDisassembledInstruction("BIT 4 H", pc, opcode, 2); break; // "BIT 4 H" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB65: outputDisassembledInstruction("BIT 4 L", pc, opcode, 2); break; // "BIT 4 L" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB66: outputDisassembledInstruction("BIT 4 [HL]", pc, opcode, 2); break; // "BIT 4 [HL]" B:2 C:12 FLAGS: Z 0 1 -
		case 0xCB67: outputDisassembledInstruction("BIT 4 A", pc, opcode, 2); break; // "BIT 4 A" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB68: outputDisassembledInstruction("BIT 5 B", pc, opcode, 2); break; // "BIT 5 B" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB69: outputDisassembledInstruction("BIT 5 C", pc, opcode, 2); break; // "BIT 5 C" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB6A: outputDisassembledInstruction("BIT 5 D", pc, opcode, 2); break; // "BIT 5 D" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB6B: outputDisassembledInstruction("BIT 5 E", pc, opcode, 2); break; // "BIT 5 E" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB6C: outputDisassembledInstruction("BIT 5 H", pc, opcode, 2); break; // "BIT 5 H" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB6D: outputDisassembledInstruction("BIT 5 L", pc, opcode, 2); break; // "BIT 5 L" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB6E: outputDisassembledInstruction("BIT 5 [HL]", pc, opcode, 2); break; // "BIT 5 [HL]" B:2 C:12 FLAGS: Z 0 1 -
		case 0xCB6F: outputDisassembledInstruction("BIT 5 A", pc, opcode, 2); break; // "BIT 5 A" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB70: outputDisassembledInstruction("BIT 6 B", pc, opcode, 2); break; // "BIT 6 B" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB71: outputDisassembledInstruction("BIT 6 C", pc, opcode, 2); break; // "BIT 6 C" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB72: outputDisassembledInstruction("BIT 6 D", pc, opcode, 2); break; // "BIT 6 D" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB73: outputDisassembledInstruction("BIT 6 E", pc, opcode, 2); break; // "BIT 6 E" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB74: outputDisassembledInstruction("BIT 6 H", pc, opcode, 2); break; // "BIT 6 H" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB75: outputDisassembledInstruction("BIT 6 L", pc, opcode, 2); break; // "BIT 6 L" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB76: outputDisassembledInstruction("BIT 6 [HL]", pc, opcode, 2); break; // "BIT 6 [HL]" B:2 C:12 FLAGS: Z 0 1 -
		case 0xCB77: outputDisassembledInstruction("BIT 6 A", pc, opcode, 2); break; // "BIT 6 A" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB78: outputDisassembledInstruction("BIT 7 B", pc, opcode, 2); break; // "BIT 7 B" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB79: outputDisassembledInstruction("BIT 7 C", pc, opcode, 2); break; // "BIT 7 C" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB7A: outputDisassembledInstruction("BIT 7 D", pc, opcode, 2); break; // "BIT 7 D" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB7B: outputDisassembledInstruction("BIT 7 E", pc, opcode, 2); break; // "BIT 7 E" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB7C: outputDisassembledInstruction("BIT 7 H", pc, opcode, 2); break; // "BIT 7 H" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB7D: outputDisassembledInstruction("BIT 7 L", pc, opcode, 2); break; // "BIT 7 L" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB7E: outputDisassembledInstruction("BIT 7 [HL]", pc, opcode, 2); break; // "BIT 7 [HL]" B:2 C:12 FLAGS: Z 0 1 -
		case 0xCB7F: outputDisassembledInstruction("BIT 7 A", pc, opcode, 2); break; // "BIT 7 A" B:2 C:8 FLAGS: Z 0 1 -
		case 0xCB80: outputDisassembledInstruction("RES 0 B", pc, opcode, 2); break; // "RES 0 B" B:2 C:8 FLAGS: - - - -
		case 0xCB81: outputDisassembledInstruction("RES 0 C", pc, opcode, 2); break; // "RES 0 C" B:2 C:8 FLAGS: - - - -
		case 0xCB82: outputDisassembledInstruction("RES 0 D", pc, opcode, 2); break; // "RES 0 D" B:2 C:8 FLAGS: - - - -
		case 0xCB83: outputDisassembledInstruction("RES 0 E", pc, opcode, 2); break; // "RES 0 E" B:2 C:8 FLAGS: - - - -
		case 0xCB84: outputDisassembledInstruction("RES 0 H", pc, opcode, 2); break; // "RES 0 H" B:2 C:8 FLAGS: - - - -
		case 0xCB85: outputDisassembledInstruction("RES 0 L", pc, opcode, 2); break; // "RES 0 L" B:2 C:8 FLAGS: - - - -
		case 0xCB86: outputDisassembledInstruction("RES 0 [HL]", pc, opcode, 2); break; // "RES 0 [HL]" B:2 C:16 FLAGS: - - - -
		case 0xCB87: outputDisassembledInstruction("RES 0 A", pc, opcode, 2); break; // "RES 0 A" B:2 C:8 FLAGS: - - - -
		case 0xCB88: outputDisassembledInstruction("RES 1 B", pc, opcode, 2); break; // "RES 1 B" B:2 C:8 FLAGS: - - - -
		case 0xCB89: outputDisassembledInstruction("RES 1 C", pc, opcode, 2); break; // "RES 1 C" B:2 C:8 FLAGS: - - - -
		case 0xCB8A: outputDisassembledInstruction("RES 1 D", pc, opcode, 2); break; // "RES 1 D" B:2 C:8 FLAGS: - - - -
		case 0xCB8B: outputDisassembledInstruction("RES 1 E", pc, opcode, 2); break; // "RES 1 E" B:2 C:8 FLAGS: - - - -
		case 0xCB8C: outputDisassembledInstruction("RES 1 H", pc, opcode, 2); break; // "RES 1 H" B:2 C:8 FLAGS: - - - -
		case 0xCB8D: outputDisassembledInstruction("RES 1 L", pc, opcode, 2); break; // "RES 1 L" B:2 C:8 FLAGS: - - - -
		case 0xCB8E: outputDisassembledInstruction("RES 1 [HL]", pc, opcode, 2); break; // "RES 1 [HL]" B:2 C:16 FLAGS: - - - -
		case 0xCB8F: outputDisassembledInstruction("RES 1 A", pc, opcode, 2); break; // "RES 1 A" B:2 C:8 FLAGS: - - - -
		case 0xCB90: outputDisassembledInstruction("RES 2 B", pc, opcode, 2); break; // "RES 2 B" B:2 C:8 FLAGS: - - - -
		case 0xCB91: outputDisassembledInstruction("RES 2 C", pc, opcode, 2); break; // "RES 2 C" B:2 C:8 FLAGS: - - - -
		case 0xCB92: outputDisassembledInstruction("RES 2 D", pc, opcode, 2); break; // "RES 2 D" B:2 C:8 FLAGS: - - - -
		case 0xCB93: outputDisassembledInstruction("RES 2 E", pc, opcode, 2); break; // "RES 2 E" B:2 C:8 FLAGS: - - - -
		case 0xCB94: outputDisassembledInstruction("RES 2 H", pc, opcode, 2); break; // "RES 2 H" B:2 C:8 FLAGS: - - - -
		case 0xCB95: outputDisassembledInstruction("RES 2 L", pc, opcode, 2); break; // "RES 2 L" B:2 C:8 FLAGS: - - - -
		case 0xCB96: outputDisassembledInstruction("RES 2 [HL]", pc, opcode, 2); break; // "RES 2 [HL]" B:2 C:16 FLAGS: - - - -
		case 0xCB97: outputDisassembledInstruction("RES 2 A", pc, opcode, 2); break; // "RES 2 A" B:2 C:8 FLAGS: - - - -
		case 0xCB98: outputDisassembledInstruction("RES 3 B", pc, opcode, 2); break; // "RES 3 B" B:2 C:8 FLAGS: - - - -
		case 0xCB99: outputDisassembledInstruction("RES 3 C", pc, opcode, 2); break; // "RES 3 C" B:2 C:8 FLAGS: - - - -
		case 0xCB9A: outputDisassembledInstruction("RES 3 D", pc, opcode, 2); break; // "RES 3 D" B:2 C:8 FLAGS: - - - -
		case 0xCB9B: outputDisassembledInstruction("RES 3 E", pc, opcode, 2); break; // "RES 3 E" B:2 C:8 FLAGS: - - - -
		case 0xCB9C: outputDisassembledInstruction("RES 3 H", pc, opcode, 2); break; // "RES 3 H" B:2 C:8 FLAGS: - - - -
		case 0xCB9D: outputDisassembledInstruction("RES 3 L", pc, opcode, 2); break; // "RES 3 L" B:2 C:8 FLAGS: - - - -
		case 0xCB9E: outputDisassembledInstruction("RES 3 [HL]", pc, opcode, 2); break; // "RES 3 [HL]" B:2 C:16 FLAGS: - - - -
		case 0xCB9F: outputDisassembledInstruction("RES 3 A", pc, opcode, 2); break; // "RES 3 A" B:2 C:8 FLAGS: - - - -
		case 0xCBA0: outputDisassembledInstruction("RES 4 B", pc, opcode, 2); break; // "RES 4 B" B:2 C:8 FLAGS: - - - -
		case 0xCBA1: outputDisassembledInstruction("RES 4 C", pc, opcode, 2); break; // "RES 4 C" B:2 C:8 FLAGS: - - - -
		case 0xCBA2: outputDisassembledInstruction("RES 4 D", pc, opcode, 2); break; // "RES 4 D" B:2 C:8 FLAGS: - - - -
		case 0xCBA3: outputDisassembledInstruction("RES 4 E", pc, opcode, 2); break; // "RES 4 E" B:2 C:8 FLAGS: - - - -
		case 0xCBA4: outputDisassembledInstruction("RES 4 H", pc, opcode, 2); break; // "RES 4 H" B:2 C:8 FLAGS: - - - -
		case 0xCBA5: outputDisassembledInstruction("RES 4 L", pc, opcode, 2); break; // "RES 4 L" B:2 C:8 FLAGS: - - - -
		case 0xCBA6: outputDisassembledInstruction("RES 4 [HL]", pc, opcode, 2); break; // "RES 4 [HL]" B:2 C:16 FLAGS: - - - -
		case 0xCBA7: outputDisassembledInstruction("RES 4 A", pc, opcode, 2); break; // "RES 4 A" B:2 C:8 FLAGS: - - - -
		case 0xCBA8: outputDisassembledInstruction("RES 5 B", pc, opcode, 2); break; // "RES 5 B" B:2 C:8 FLAGS: - - - -
		case 0xCBA9: outputDisassembledInstruction("RES 5 C", pc, opcode, 2); break; // "RES 5 C" B:2 C:8 FLAGS: - - - -
		case 0xCBAA: outputDisassembledInstruction("RES 5 D", pc, opcode, 2); break; // "RES 5 D" B:2 C:8 FLAGS: - - - -
		case 0xCBAB: outputDisassembledInstruction("RES 5 E", pc, opcode, 2); break; // "RES 5 E" B:2 C:8 FLAGS: - - - -
		case 0xCBAC: outputDisassembledInstruction("RES 5 H", pc, opcode, 2); break; // "RES 5 H" B:2 C:8 FLAGS: - - - -
		case 0xCBAD: outputDisassembledInstruction("RES 5 L", pc, opcode, 2); break; // "RES 5 L" B:2 C:8 FLAGS: - - - -
		case 0xCBAE: outputDisassembledInstruction("RES 5 [HL]", pc, opcode, 2); break; // "RES 5 [HL]" B:2 C:16 FLAGS: - - - -
		case 0xCBAF: outputDisassembledInstruction("RES 5 A", pc, opcode, 2); break; // "RES 5 A" B:2 C:8 FLAGS: - - - -
		case 0xCBB0: outputDisassembledInstruction("RES 6 B", pc, opcode, 2); break; // "RES 6 B" B:2 C:8 FLAGS: - - - -
		case 0xCBB1: outputDisassembledInstruction("RES 6 C", pc, opcode, 2); break; // "RES 6 C" B:2 C:8 FLAGS: - - - -
		case 0xCBB2: outputDisassembledInstruction("RES 6 D", pc, opcode, 2); break; // "RES 6 D" B:2 C:8 FLAGS: - - - -
		case 0xCBB3: outputDisassembledInstruction("RES 6 E", pc, opcode, 2); break; // "RES 6 E" B:2 C:8 FLAGS: - - - -
		case 0xCBB4: outputDisassembledInstruction("RES 6 H", pc, opcode, 2); break; // "RES 6 H" B:2 C:8 FLAGS: - - - -
		case 0xCBB5: outputDisassembledInstruction("RES 6 L", pc, opcode, 2); break; // "RES 6 L" B:2 C:8 FLAGS: - - - -
		case 0xCBB6: outputDisassembledInstruction("RES 6 [HL]", pc, opcode, 2); break; // "RES 6 [HL]" B:2 C:16 FLAGS: - - - -
		case 0xCBB7: outputDisassembledInstruction("RES 6 A", pc, opcode, 2); break; // "RES 6 A" B:2 C:8 FLAGS: - - - -
		case 0xCBB8: outputDisassembledInstruction("RES 7 B", pc, opcode, 2); break; // "RES 7 B" B:2 C:8 FLAGS: - - - -
		case 0xCBB9: outputDisassembledInstruction("RES 7 C", pc, opcode, 2); break; // "RES 7 C" B:2 C:8 FLAGS: - - - -
		case 0xCBBA: outputDisassembledInstruction("RES 7 D", pc, opcode, 2); break; // "RES 7 D" B:2 C:8 FLAGS: - - - -
		case 0xCBBB: outputDisassembledInstruction("RES 7 E", pc, opcode, 2); break; // "RES 7 E" B:2 C:8 FLAGS: - - - -
		case 0xCBBC: outputDisassembledInstruction("RES 7 H", pc, opcode, 2); break; // "RES 7 H" B:2 C:8 FLAGS: - - - -
		case 0xCBBD: outputDisassembledInstruction("RES 7 L", pc, opcode, 2); break; // "RES 7 L" B:2 C:8 FLAGS: - - - -
		case 0xCBBE: outputDisassembledInstruction("RES 7 [HL]", pc, opcode, 2); break; // "RES 7 [HL]" B:2 C:16 FLAGS: - - - -
		case 0xCBBF: outputDisassembledInstruction("RES 7 A", pc, opcode, 2); break; // "RES 7 A" B:2 C:8 FLAGS: - - - -
		case 0xCBC0: outputDisassembledInstruction("SET 0 B", pc, opcode, 2); break; // "SET 0 B" B:2 C:8 FLAGS: - - - -
		case 0xCBC1: outputDisassembledInstruction("SET 0 C", pc, opcode, 2); break; // "SET 0 C" B:2 C:8 FLAGS: - - - -
		case 0xCBC2: outputDisassembledInstruction("SET 0 D", pc, opcode, 2); break; // "SET 0 D" B:2 C:8 FLAGS: - - - -
		case 0xCBC3: outputDisassembledInstruction("SET 0 E", pc, opcode, 2); break; // "SET 0 E" B:2 C:8 FLAGS: - - - -
		case 0xCBC4: outputDisassembledInstruction("SET 0 H", pc, opcode, 2); break; // "SET 0 H" B:2 C:8 FLAGS: - - - -
		case 0xCBC5: outputDisassembledInstruction("SET 0 L", pc, opcode, 2); break; // "SET 0 L" B:2 C:8 FLAGS: - - - -
		case 0xCBC6: outputDisassembledInstruction("SET 0 [HL]", pc, opcode, 2); break; // "SET 0 [HL]" B:2 C:16 FLAGS: - - - -
		case 0xCBC7: outputDisassembledInstruction("SET 0 A", pc, opcode, 2); break; // "SET 0 A" B:2 C:8 FLAGS: - - - -
		case 0xCBC8: outputDisassembledInstruction("SET 1 B", pc, opcode, 2); break; // "SET 1 B" B:2 C:8 FLAGS: - - - -
		case 0xCBC9: outputDisassembledInstruction("SET 1 C", pc, opcode, 2); break; // "SET 1 C" B:2 C:8 FLAGS: - - - -
		case 0xCBCA: outputDisassembledInstruction("SET 1 D", pc, opcode, 2); break; // "SET 1 D" B:2 C:8 FLAGS: - - - -
		case 0xCBCB: outputDisassembledInstruction("SET 1 E", pc, opcode, 2); break; // "SET 1 E" B:2 C:8 FLAGS: - - - -
		case 0xCBCC: outputDisassembledInstruction("SET 1 H", pc, opcode, 2); break; // "SET 1 H" B:2 C:8 FLAGS: - - - -
		case 0xCBCD: outputDisassembledInstruction("SET 1 L", pc, opcode, 2); break; // "SET 1 L" B:2 C:8 FLAGS: - - - -
		case 0xCBCE: outputDisassembledInstruction("SET 1 [HL]", pc, opcode, 2); break; // "SET 1 [HL]" B:2 C:16 FLAGS: - - - -
		case 0xCBCF: outputDisassembledInstruction("SET 1 A", pc, opcode, 2); break; // "SET 1 A" B:2 C:8 FLAGS: - - - -
		case 0xCBD0: outputDisassembledInstruction("SET 2 B", pc, opcode, 2); break; // "SET 2 B" B:2 C:8 FLAGS: - - - -
		case 0xCBD1: outputDisassembledInstruction("SET 2 C", pc, opcode, 2); break; // "SET 2 C" B:2 C:8 FLAGS: - - - -
		case 0xCBD2: outputDisassembledInstruction("SET 2 D", pc, opcode, 2); break; // "SET 2 D" B:2 C:8 FLAGS: - - - -
		case 0xCBD3: outputDisassembledInstruction("SET 2 E", pc, opcode, 2); break; // "SET 2 E" B:2 C:8 FLAGS: - - - -
		case 0xCBD4: outputDisassembledInstruction("SET 2 H", pc, opcode, 2); break; // "SET 2 H" B:2 C:8 FLAGS: - - - -
		case 0xCBD5: outputDisassembledInstruction("SET 2 L", pc, opcode, 2); break; // "SET 2 L" B:2 C:8 FLAGS: - - - -
		case 0xCBD6: outputDisassembledInstruction("SET 2 [HL]", pc, opcode, 2); break; // "SET 2 [HL]" B:2 C:16 FLAGS: - - - -
		case 0xCBD7: outputDisassembledInstruction("SET 2 A", pc, opcode, 2); break; // "SET 2 A" B:2 C:8 FLAGS: - - - -
		case 0xCBD8: outputDisassembledInstruction("SET 3 B", pc, opcode, 2); break; // "SET 3 B" B:2 C:8 FLAGS: - - - -
		case 0xCBD9: outputDisassembledInstruction("SET 3 C", pc, opcode, 2); break; // "SET 3 C" B:2 C:8 FLAGS: - - - -
		case 0xCBDA: outputDisassembledInstruction("SET 3 D", pc, opcode, 2); break; // "SET 3 D" B:2 C:8 FLAGS: - - - -
		case 0xCBDB: outputDisassembledInstruction("SET 3 E", pc, opcode, 2); break; // "SET 3 E" B:2 C:8 FLAGS: - - - -
		case 0xCBDC: outputDisassembledInstruction("SET 3 H", pc, opcode, 2); break; // "SET 3 H" B:2 C:8 FLAGS: - - - -
		case 0xCBDD: outputDisassembledInstruction("SET 3 L", pc, opcode, 2); break; // "SET 3 L" B:2 C:8 FLAGS: - - - -
		case 0xCBDE: outputDisassembledInstruction("SET 3 [HL]", pc, opcode, 2); break; // "SET 3 [HL]" B:2 C:16 FLAGS: - - - -
		case 0xCBDF: outputDisassembledInstruction("SET 3 A", pc, opcode, 2); break; // "SET 3 A" B:2 C:8 FLAGS: - - - -
		case 0xCBE0: outputDisassembledInstruction("SET 4 B", pc, opcode, 2); break; // "SET 4 B" B:2 C:8 FLAGS: - - - -
		case 0xCBE1: outputDisassembledInstruction("SET 4 C", pc, opcode, 2); break; // "SET 4 C" B:2 C:8 FLAGS: - - - -
		case 0xCBE2: outputDisassembledInstruction("SET 4 D", pc, opcode, 2); break; // "SET 4 D" B:2 C:8 FLAGS: - - - -
		case 0xCBE3: outputDisassembledInstruction("SET 4 E", pc, opcode, 2); break; // "SET 4 E" B:2 C:8 FLAGS: - - - -
		case 0xCBE4: outputDisassembledInstruction("SET 4 H", pc, opcode, 2); break; // "SET 4 H" B:2 C:8 FLAGS: - - - -
		case 0xCBE5: outputDisassembledInstruction("SET 4 L", pc, opcode, 2); break; // "SET 4 L" B:2 C:8 FLAGS: - - - -
		case 0xCBE6: outputDisassembledInstruction("SET 4 [HL]", pc, opcode, 2); break; // "SET 4 [HL]" B:2 C:16 FLAGS: - - - -
		case 0xCBE7: outputDisassembledInstruction("SET 4 A", pc, opcode, 2); break; // "SET 4 A" B:2 C:8 FLAGS: - - - -
		case 0xCBE8: outputDisassembledInstruction("SET 5 B", pc, opcode, 2); break; // "SET 5 B" B:2 C:8 FLAGS: - - - -
		case 0xCBE9: outputDisassembledInstruction("SET 5 C", pc, opcode, 2); break; // "SET 5 C" B:2 C:8 FLAGS: - - - -
		case 0xCBEA: outputDisassembledInstruction("SET 5 D", pc, opcode, 2); break; // "SET 5 D" B:2 C:8 FLAGS: - - - -
		case 0xCBEB: outputDisassembledInstruction("SET 5 E", pc, opcode, 2); break; // "SET 5 E" B:2 C:8 FLAGS: - - - -
		case 0xCBEC: outputDisassembledInstruction("SET 5 H", pc, opcode, 2); break; // "SET 5 H" B:2 C:8 FLAGS: - - - -
		case 0xCBED: outputDisassembledInstruction("SET 5 L", pc, opcode, 2); break; // "SET 5 L" B:2 C:8 FLAGS: - - - -
		case 0xCBEE: outputDisassembledInstruction("SET 5 [HL]", pc, opcode, 2); break; // "SET 5 [HL]" B:2 C:16 FLAGS: - - - -
		case 0xCBEF: outputDisassembledInstruction("SET 5 A", pc, opcode, 2); break; // "SET 5 A" B:2 C:8 FLAGS: - - - -
		case 0xCBF0: outputDisassembledInstruction("SET 6 B", pc, opcode, 2); break; // "SET 6 B" B:2 C:8 FLAGS: - - - -
		case 0xCBF1: outputDisassembledInstruction("SET 6 C", pc, opcode, 2); break; // "SET 6 C" B:2 C:8 FLAGS: - - - -
		case 0xCBF2: outputDisassembledInstruction("SET 6 D", pc, opcode, 2); break; // "SET 6 D" B:2 C:8 FLAGS: - - - -
		case 0xCBF3: outputDisassembledInstruction("SET 6 E", pc, opcode, 2); break; // "SET 6 E" B:2 C:8 FLAGS: - - - -
		case 0xCBF4: outputDisassembledInstruction("SET 6 H", pc, opcode, 2); break; // "SET 6 H" B:2 C:8 FLAGS: - - - -
		case 0xCBF5: outputDisassembledInstruction("SET 6 L", pc, opcode, 2); break; // "SET 6 L" B:2 C:8 FLAGS: - - - -
		case 0xCBF6: outputDisassembledInstruction("SET 6 [HL]", pc, opcode, 2); break; // "SET 6 [HL]" B:2 C:16 FLAGS: - - - -
		case 0xCBF7: outputDisassembledInstruction("SET 6 A", pc, opcode, 2); break; // "SET 6 A" B:2 C:8 FLAGS: - - - -
		case 0xCBF8: outputDisassembledInstruction("SET 7 B", pc, opcode, 2); break; // "SET 7 B" B:2 C:8 FLAGS: - - - -
		case 0xCBF9: outputDisassembledInstruction("SET 7 C", pc, opcode, 2); break; // "SET 7 C" B:2 C:8 FLAGS: - - - -
		case 0xCBFA: outputDisassembledInstruction("SET 7 D", pc, opcode, 2); break; // "SET 7 D" B:2 C:8 FLAGS: - - - -
		case 0xCBFB: outputDisassembledInstruction("SET 7 E", pc, opcode, 2); break; // "SET 7 E" B:2 C:8 FLAGS: - - - -
		case 0xCBFC: outputDisassembledInstruction("SET 7 H", pc, opcode, 2); break; // "SET 7 H" B:2 C:8 FLAGS: - - - -
		case 0xCBFD: outputDisassembledInstruction("SET 7 L", pc, opcode, 2); break; // "SET 7 L" B:2 C:8 FLAGS: - - - -
		case 0xCBFE: outputDisassembledInstruction("SET 7 [HL]", pc, opcode, 2); break; // "SET 7 [HL]" B:2 C:16 FLAGS: - - - -
		case 0xCBFF: outputDisassembledInstruction("SET 7 A", pc, opcode, 2); break; // "SET 7 A" B:2 C:8 FLAGS: - - - -
	}
}

void outputDisassembledInstruction(const char* instructionName, int pc, uint8_t *opcode, int totalOpBytes)
{
	/* Ideal format
		0000 00       NOP
		0001 c3 d4 18 JMP    $18d4
	*/

	// print program counter address
	printf("%04x", pc);
	printf(" ");

	// print address values
	if (totalOpBytes == 3)
	{
		printf("%02x ", opcode[0]);
		printf("%02x ", opcode[1]);
		printf("%02x ", opcode[2]);
	}
	else if (totalOpBytes == 2)
	{
		printf("%02x ", opcode[0]);
		printf("%02x ", opcode[1]);
		printf("   ");
	}
	else
	{
		printf("%02x ", opcode[0]);
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
		printf("%02x", opcode[2]);
		printf("%02x", opcode[1]);
	}
	else if (totalOpBytes == 2)
	{
		printf("$0x");
		printf("%02x", opcode[1]);
	}

	// new line
	std::cout << "\n";
}