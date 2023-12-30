#include "Cpu.h"
#include "GameBoy.h"
#include "Defines.h"
#include "Logger.h"

#include <fstream>
#include <filesystem>
#include <sstream>

namespace Core
{
	Cpu::Cpu(GameBoy& gb) : gb(gb)
	{
		// Check if the log file already exists
		if (std::filesystem::exists("cpu.txt")) {
			// If it exists, delete it
			std::filesystem::remove("cpu.txt");
		}
	}

	Cpu::~Cpu() {}

	int linecount = 0;
	std::ostringstream logBuffer;

	void Cpu::Clock()
	{
		Cpu::m_TotalCycles++;

		processTimers();

		// Each instruction takes a certain amount of cycles to complete so
		// if there are still cycles remaining then we shoud just decrement 
		// the cycles and return;
		if (m_cycles > 0)
		{
			m_InstructionCompleted = false;

			m_cycles--;
			return;
		}

		linecount++;

		// read opcode from memory
		uint8_t* opcode = &gb.ReadFromMemoryMap(State.PC);

		//Disassemble(opcode, State.PC);

		if (false)
		{
			logBuffer << std::hex << std::setfill('0') << std::uppercase <<
				"A: " << std::setw(2) << static_cast<int>(State.A) <<
				" F: " << std::setw(2) << static_cast<int>(State.F) <<
				" B: " << std::setw(2) << static_cast<int>(State.B) <<
				" C: " << std::setw(2) << static_cast<int>(State.C) <<
				" D: " << std::setw(2) << static_cast<int>(State.D) <<
				" E: " << std::setw(2) << static_cast<int>(State.E) <<
				" H: " << std::setw(2) << static_cast<int>(State.H) <<
				" L: " << std::setw(2) << static_cast<int>(State.L) <<
				" SP: " << std::setw(4) << static_cast<int>(State.SP) <<
				" PC: 00:" << std::setw(4) << static_cast<int>(State.PC) <<
				" ("
				<< std::setw(2) << static_cast<int>(opcode[0]) << " "
				<< std::setw(2) << static_cast<int>(opcode[1]) << " "
				<< std::setw(2) << static_cast<int>(opcode[2]) << " "
				<< std::setw(2) << static_cast<int>(opcode[3]) << ")" << std::endl;

			if (linecount >= 10000)
			{
				std::ofstream outFile("cpu.txt", std::ios::out | std::ios::app);
				outFile << logBuffer.str();
				logBuffer.str("");  // Clear the buffer
				linecount = 0;      // Reset line count
				outFile.close();
			}
		}


		if (!m_isHalted)
		{
			// increment program counter
			State.PC++;

			switch (*opcode)
			{
				/********************************************************************************************
					Misc / Control Instructions
				*********************************************************************************************/

				// "NOP" B:1 C:4 FLAGS: - - - -
				case 0x00: m_cycles = 4; break;

					// "STOP n8" B:2 C:4 FLAGS: - - - -
				case 0x10: State.PC++; m_cycles = 4; break;

					// "HALT" B:1 C:4 FLAGS: - - - -
				case 0x76: m_isHalted = true; m_cycles = 4;	break;

					// "PREFIX" B:1 C:4 FLAGS: - - - -
				case 0xCB: process16bitInstruction((opcode[0] << 8) | (opcode[1]), State); State.PC++; break;

					// "DI" B:1 C:4 FLAGS: - - - -
				case 0xF3: m_interruptMasterFlag = false; m_cycles = 4; break;

					// "EI" B:1 C:4 FLAGS: - - - -
				case 0xFB: m_interruptMasterFlag = true; m_cycles = 4; break;


					/********************************************************************************************
						Jumps/Calls
					*********************************************************************************************/

					// "JR e8" B:2 C:12 FLAGS: - - - -
				case 0x18:
				{
					State.PC += (int8_t)opcode[1] + 1;

					m_cycles = 12;
					break;
				}

				// "JR NZ e8" B:2 C:12/8 FLAGS: - - - -
				case 0x20:
				{
					// note: "e8" in the description refers to a signed char
					int8_t offset = opcode[1];
					if (GetCPUFlag(FLAG_ZERO) == 0)
					{
						State.PC += offset + 1;
						m_cycles = 12;
					}
					else
					{
						State.PC++;
						m_cycles = 8;
					}

					break;
				}

				// "JR Z e8" B:2 C:12/8 FLAGS: - - - -
				case 0x28:
				{
					// note: "e8" in the description refers to a signed char
					int8_t offset = opcode[1];
					if (GetCPUFlag(FLAG_ZERO) == 1)
					{
						State.PC += offset + 1;
						m_cycles = 12;
					}
					else
					{
						State.PC++;
						m_cycles = 8;
					}

					break;
				}

				// "JR NC e8" B:2 C:12/8 FLAGS: - - - -
				case 0x30:
				{
					// note: "s8" in the description refers to a signed char
					int8_t offset = opcode[1];
					if (!GetCPUFlag(FLAG_CARRY))
					{
						State.PC += offset + 1;
						m_cycles = 12;
					}
					else
					{
						State.PC++;
						m_cycles = 8;
					}

					break;
				}

				// "JR C e8" B:2 C:12/8 FLAGS: - - - -
				case 0x38:
				{
					// note: "s8" in the description refers to a signed char
					int8_t offset = opcode[1];
					if (GetCPUFlag(FLAG_CARRY))
					{
						State.PC += offset + 1;
						m_cycles = 12;
					}
					else
					{
						State.PC++;
						m_cycles = 8;
					}

					break;
				}

				// "RET NZ" B:1 C:20/8 FLAGS: - - - -
				case 0xC0:
				{
					if (!GetCPUFlag(FLAG_ZERO))
					{
						State.PC = popSP();
						m_cycles = 20;
						break;
					}

					m_cycles = 8;
					break;
				}

				// "JP NZ a16" B:3 C:16/12 FLAGS: - - - -
				case 0xC2:
				{
					if (!GetCPUFlag(FLAG_ZERO))
					{
						uint16_t offset = (opcode[2] << 8) | (opcode[1]);
						State.PC = offset;

						m_cycles = 16;
						break;
					}

					State.PC += 2;

					m_cycles = 12;
					break;
				}

						 // "JP a16" B:3 C:16 FLAGS: - - - -
				case 0xC3:
				{
					uint16_t offset = (opcode[2] << 8) | (opcode[1]);
					State.PC = offset;

					m_cycles = 16;
					break;
				}

				// "CALL NZ a16" B:3 C:24/12 FLAGS: - - - -
				case 0xC4:
				{
					if (!GetCPUFlag(FLAG_ZERO))
					{
						pushSP(State.PC += 2);
						State.PC = (opcode[2] << 8) | (opcode[1]);

						m_cycles = 24;
						break;
					}

					State.PC += 2;

					m_cycles = 12;
					break;
				}

				// "RST $00" B:1 C:16 FLAGS: - - - -
				case 0xC7:
				{
					pushSP(State.PC);
					State.PC = 0x00;

					m_cycles = 16;
					break;
				}

				// "RET Z" B:1 C:20/8 FLAGS: - - - -
				case 0xC8:
				{
					if (GetCPUFlag(FLAG_ZERO))
					{
						State.PC = popSP();
						m_cycles = 20;
						break;
					}

					m_cycles = 8;
					break;
				}

				// "RET" B:1 C:16 FLAGS: - - - -
				case 0xC9:
				{
					State.PC = popSP();

					m_cycles = 16;
					break;
				}

				// "JP Z a16" B:3 C:16/12 FLAGS: - - - -
				case 0xCA:
				{
					if (GetCPUFlag(FLAG_ZERO))
					{
						uint16_t offset = (opcode[2] << 8) | (opcode[1]);
						State.PC = offset;

						m_cycles = 16;
						break;
					}

					State.PC += 2;

					m_cycles = 12;
					break;
				}

				// "CALL Z a16" B:3 C:24/12 FLAGS: - - - -
				case 0xCC:
				{
					if (GetCPUFlag(FLAG_ZERO))
					{
						pushSP(State.PC += 2);
						State.PC = (opcode[2] << 8) | (opcode[1]);
						m_cycles = 24;
						break;
					}

					State.PC += 2;

					m_cycles = 12;
					break;
				}

				// "CALL a16" B:3 C:24 FLAGS: - - - -
				case 0xCD:
				{
					pushSP(State.PC += 2);
					State.PC = (opcode[2] << 8) | (opcode[1]);

					m_cycles = 24;
					break;
				}

				// "RST $08" B:1 C:16 FLAGS: - - - -
				case 0xCF:
				{
					pushSP(State.PC);
					State.PC = 0x08;

					m_cycles = 16;
					break;
				}

				// "RET NC" B:1 C:20/8 FLAGS: - - - -
				case 0xD0:
				{
					if (!GetCPUFlag(FLAG_CARRY))
					{
						State.PC = popSP();
						m_cycles = 20;
						break;
					}

					m_cycles = 8;
					break;
				}

				// "JP NC a16" B:3 C:16/12 FLAGS: - - - -
				case 0xD2:
				{
					if (!GetCPUFlag(FLAG_CARRY))
					{
						uint16_t offset = (opcode[2] << 8) | (opcode[1]);
						State.PC = offset;

						m_cycles = 16;
						break;
					}

					State.PC += 2;

					m_cycles = 12;
					break;
				}

				// "CALL NC a16" B:3 C:24/12 FLAGS: - - - -
				case 0xD4:
				{
					if (!GetCPUFlag(FLAG_CARRY))
					{
						pushSP(State.PC += 2);
						State.PC = (opcode[2] << 8) | (opcode[1]);

						m_cycles = 24;
						break;
					}

					State.PC += 2;

					m_cycles = 12;
					break;
				}

				// "RST $10" B:1 C:16 FLAGS: - - - -
				case 0xD7:
				{
					pushSP(State.PC);
					State.PC = 0x10;

					m_cycles = 16;
					break;
				}

				// "RET C" B:1 C:20/8 FLAGS: - - - -
				case 0xD8:
				{
					if (GetCPUFlag(FLAG_CARRY))
					{
						State.PC = popSP();
						m_cycles = 20;
						break;
					}

					m_cycles = 8;
					break;
				}

				// "RETI" B:1 C:16 FLAGS: - - - -
				case 0xD9:
				{
					State.PC = popSP();
					m_interruptMasterFlag = true;

					m_cycles = 16;
					break;
				}

				// "JP C a16" B:3 C:16/12 FLAGS: - - - -
				case 0xDA:
				{
					if (GetCPUFlag(FLAG_CARRY))
					{
						uint16_t offset = (opcode[2] << 8) | (opcode[1]);
						State.PC = offset;

						m_cycles = 16;
						break;
					}

					State.PC += 2;

					m_cycles = 12;
					break;
				}

						 // "CALL C a16" B:3 C:24/12 FLAGS: - - - -
				case 0xDC:
				{
					if (GetCPUFlag(FLAG_CARRY))
					{
						pushSP(State.PC += 2);
						State.PC = (opcode[2] << 8) | (opcode[1]);

						m_cycles = 24;
						break;
					}

					State.PC += 2;

					m_cycles = 12;
					break;
				}

						 // "RST $18" B:1 C:16 FLAGS: - - - -
				case 0xDF:
				{
					pushSP(State.PC);
					State.PC = 0x18;

					m_cycles = 16;
					break;
				}

				// "RST $20" B:1 C:16 FLAGS: - - - -
				case 0xE7:
				{
					pushSP(State.PC);
					State.PC = 0x20;

					m_cycles = 16;
					break;
				}

				// "JP HL" B:1 C:4 FLAGS: - - - -
				case 0xE9:
				{
					State.PC = State.HL;

					m_cycles = 4;
					break;
				}

				// "RST $28" B:1 C:16 FLAGS: - - - -
				case 0xEF:
				{
					pushSP(State.PC);
					State.PC = 0x28;

					m_cycles = 16;
					break;
				}


				// "RST $30" B:1 C:16 FLAGS: - - - -
				case 0xF7:
				{
					pushSP(State.PC);
					State.PC = 0x30;

					m_cycles = 16;
					break;
				}


				// "RST $38" B:1 C:16 FLAGS: - - - -
				case 0xFF:
				{
					pushSP(State.PC);
					State.PC = 0x38;

					m_cycles = 16;
					break;
				}


				/********************************************************************************************
					8-bit Load Instructions
				*********************************************************************************************/

				// "LD [addr] reg" B:1 C:8 FLAGS: - - - -
				case 0x02: instruction_ld_addr_reg(State.BC, State.A); m_cycles = 8; break;
				case 0x12: instruction_ld_addr_reg(State.DE, State.A); m_cycles = 8; break;

					// "LD reg [addr]" B:1 C:8 FLAGS: - - - -
				case 0x0A: instruction_ld_reg_addr(State.A, State.BC); m_cycles = 8; break;
				case 0x1A: instruction_ld_reg_addr(State.A, State.DE); m_cycles = 8; break;
				case 0x46: instruction_ld_reg_addr(State.B, State.HL); m_cycles = 8; break;
				case 0x4E: instruction_ld_reg_addr(State.C, State.HL); m_cycles = 8; break;
				case 0x56: instruction_ld_reg_addr(State.D, State.HL); m_cycles = 8; break;
				case 0x5E: instruction_ld_reg_addr(State.E, State.HL); m_cycles = 8; break;
				case 0x66: instruction_ld_reg_addr(State.H, State.HL); m_cycles = 8; break;
				case 0x6E: instruction_ld_reg_addr(State.L, State.HL); m_cycles = 8; break;
				case 0x7E: instruction_ld_reg_addr(State.A, State.HL); m_cycles = 8; break;

					// "LD reg n8" B:2 C:8 FLAGS: - - - -
				case 0x06: instruction_ld_reg_value(State.B, opcode[1]); State.PC++; m_cycles = 8; break;
				case 0x0E: instruction_ld_reg_value(State.C, opcode[1]); State.PC++; m_cycles = 8; break;
				case 0x16: instruction_ld_reg_value(State.D, opcode[1]); State.PC++; m_cycles = 8; break;
				case 0x1E: instruction_ld_reg_value(State.E, opcode[1]); State.PC++; m_cycles = 8; break;
				case 0x26: instruction_ld_reg_value(State.H, opcode[1]); State.PC++; m_cycles = 8; break;
				case 0x2E: instruction_ld_reg_value(State.L, opcode[1]); State.PC++; m_cycles = 8; break;


					// "LD A [HL-]" B:1 C:8 FLAGS: - - - -
				case 0x3A:
				{
					State.A = gb.ReadFromMemoryMap(State.HL);
					State.HL--;

					m_cycles = 8;
					break;
				}

				// "LD [HL+] A" B:1 C:8 FLAGS: - - - -
				case 0x22:
				{
					gb.WriteToMemoryMap(State.HL, State.A);
					State.HL++;

					m_cycles = 8;
					break;
				}

				// "LD A [HL+]" B:1 C:8 FLAGS: - - - -
				case 0x2A:
				{
					State.A = gb.ReadFromMemoryMap(State.HL);
					State.HL++;

					m_cycles = 8;
					break;
				}

				// "LD [HL-] A" B:1 C:8 FLAGS: - - - -
				case 0x32:
				{
					gb.WriteToMemoryMap(State.HL, State.A);
					State.HL--;

					m_cycles = 8;
					break;
				}

				// "LD [HL] n8" B:2 C:12 FLAGS: - - - -
				case 0x36:
				{
					gb.WriteToMemoryMap(State.HL, opcode[1]);
					State.PC++;

					m_cycles = 12;
					break;
				}

				// "LD A n8" B:2 C:8 FLAGS: - - - -
				case 0x3E:
				{
					State.A = opcode[1];
					State.PC++;

					m_cycles = 8;
					break;
				}

				// "LD reg reg" B:1 C:4 FLAGS: - - - -
				case 0x40: instruction_ld_reg_value(State.B, State.B); m_cycles = 4; break;
				case 0x41: instruction_ld_reg_value(State.B, State.C); m_cycles = 4; break;
				case 0x42: instruction_ld_reg_value(State.B, State.D); m_cycles = 4; break;
				case 0x43: instruction_ld_reg_value(State.B, State.E); m_cycles = 4; break;
				case 0x44: instruction_ld_reg_value(State.B, State.H); m_cycles = 4; break;
				case 0x45: instruction_ld_reg_value(State.B, State.L); m_cycles = 4; break;
				case 0x47: instruction_ld_reg_value(State.B, State.A); m_cycles = 4; break;

				case 0x48: instruction_ld_reg_value(State.C, State.B); m_cycles = 4; break;
				case 0x49: instruction_ld_reg_value(State.C, State.C); m_cycles = 4; break;
				case 0x4A: instruction_ld_reg_value(State.C, State.D); m_cycles = 4; break;
				case 0x4B: instruction_ld_reg_value(State.C, State.E); m_cycles = 4; break;
				case 0x4C: instruction_ld_reg_value(State.C, State.H); m_cycles = 4; break;
				case 0x4D: instruction_ld_reg_value(State.C, State.L); m_cycles = 4; break;
				case 0x4F: instruction_ld_reg_value(State.C, State.A); m_cycles = 4; break;

				case 0x50: instruction_ld_reg_value(State.D, State.B); m_cycles = 4; break;
				case 0x51: instruction_ld_reg_value(State.D, State.C); m_cycles = 4; break;
				case 0x52: instruction_ld_reg_value(State.D, State.D); m_cycles = 4; break;
				case 0x53: instruction_ld_reg_value(State.D, State.E); m_cycles = 4; break;
				case 0x54: instruction_ld_reg_value(State.D, State.H); m_cycles = 4; break;
				case 0x55: instruction_ld_reg_value(State.D, State.L); m_cycles = 4; break;
				case 0x57: instruction_ld_reg_value(State.D, State.A); m_cycles = 4; break;

				case 0x58: instruction_ld_reg_value(State.E, State.B); m_cycles = 4; break;
				case 0x59: instruction_ld_reg_value(State.E, State.C); m_cycles = 4; break;
				case 0x5A: instruction_ld_reg_value(State.E, State.D); m_cycles = 4; break;
				case 0x5B: instruction_ld_reg_value(State.E, State.E); m_cycles = 4; break;
				case 0x5C: instruction_ld_reg_value(State.E, State.H); m_cycles = 4; break;
				case 0x5D: instruction_ld_reg_value(State.E, State.L); m_cycles = 4; break;
				case 0x5F: instruction_ld_reg_value(State.E, State.A); m_cycles = 4; break;

				case 0x60: instruction_ld_reg_value(State.H, State.B); m_cycles = 4; break;
				case 0x61: instruction_ld_reg_value(State.H, State.C); m_cycles = 4; break;
				case 0x62: instruction_ld_reg_value(State.H, State.D); m_cycles = 4; break;
				case 0x63: instruction_ld_reg_value(State.H, State.E); m_cycles = 4; break;
				case 0x64: instruction_ld_reg_value(State.H, State.H); m_cycles = 4; break;
				case 0x65: instruction_ld_reg_value(State.H, State.L); m_cycles = 4; break;
				case 0x67: instruction_ld_reg_value(State.H, State.A); m_cycles = 4; break;

				case 0x68: instruction_ld_reg_value(State.L, State.B); m_cycles = 4; break;
				case 0x69: instruction_ld_reg_value(State.L, State.C); m_cycles = 4; break;
				case 0x6A: instruction_ld_reg_value(State.L, State.D); m_cycles = 4; break;
				case 0x6B: instruction_ld_reg_value(State.L, State.E); m_cycles = 4; break;
				case 0x6C: instruction_ld_reg_value(State.L, State.H); m_cycles = 4; break;
				case 0x6D: instruction_ld_reg_value(State.L, State.L); m_cycles = 4; break;
				case 0x6F: instruction_ld_reg_value(State.L, State.A); m_cycles = 4; break;

				case 0x78: instruction_ld_reg_value(State.A, State.B); m_cycles = 4; break;
				case 0x79: instruction_ld_reg_value(State.A, State.C); m_cycles = 4; break;
				case 0x7A: instruction_ld_reg_value(State.A, State.D); m_cycles = 4; break;
				case 0x7B: instruction_ld_reg_value(State.A, State.E); m_cycles = 4; break;
				case 0x7C: instruction_ld_reg_value(State.A, State.H); m_cycles = 4; break;
				case 0x7D: instruction_ld_reg_value(State.A, State.L); m_cycles = 4; break;
				case 0x7F: instruction_ld_reg_value(State.A, State.A); m_cycles = 4; break;

					// "LD [HL] reg" B:1 C:8 FLAGS: - - - -
				case 0x70: instruction_ld_addr_reg(State.HL, State.B); m_cycles = 8; break;
				case 0x71: instruction_ld_addr_reg(State.HL, State.C); m_cycles = 8; break;
				case 0x72: instruction_ld_addr_reg(State.HL, State.D); m_cycles = 8; break;
				case 0x73: instruction_ld_addr_reg(State.HL, State.E); m_cycles = 8; break;
				case 0x74: instruction_ld_addr_reg(State.HL, State.H); m_cycles = 8; break;
				case 0x75: instruction_ld_addr_reg(State.HL, State.L); m_cycles = 8; break;
				case 0x77: instruction_ld_addr_reg(State.HL, State.A); m_cycles = 8; break;



					// "LDH [a8] A" B:2 C:12 FLAGS: - - - -
				case 0xE0:
				{
					uint8_t offset = opcode[1];
					gb.WriteToMemoryMap(0xFF00 + offset, State.A);
					State.PC++;

					m_cycles = 12;
					break;
				}

				// "LD [C] A" B:1 C:8 FLAGS: - - - -
				case 0xE2:
				{
					gb.WriteToMemoryMap(0xFF00 + State.C, State.A);

					m_cycles = 8;
					break;
				}

				// "LD [a16] A" B:3 C:16 FLAGS: - - - -
				case 0xEA:
				{
					gb.WriteToMemoryMap((opcode[2] << 8) | (opcode[1]), State.A);
					State.PC += 2;

					m_cycles = 16;
					break;
				}

				// "LDH A [a8]" B:2 C:12 FLAGS: - - - -
				case 0xF0:
				{
					// just a temp hack to test the cpu output
					//uint8_t test = opcode[1];
					//if (opcode[1] == 0x44)
					//	State.A = 0x90;
					//else
						State.A = gb.ReadFromMemoryMap(0xFF00 + opcode[1]);

					State.PC++;

					m_cycles = 12;
					break;
				}

				// "LD A [C]" B:1 C:8 FLAGS: - - - -
				case 0xF2:
				{
					State.A = gb.ReadFromMemoryMap(0xFF00 + State.C);

					m_cycles = 8;
					break;
				}

				// "LD A [a16]" B:3 C:16 FLAGS: - - - -
				case 0xFA:
				{
					State.A = gb.ReadFromMemoryMap((opcode[2] << 8) | (opcode[1]));
					State.PC += 2;

					m_cycles = 16;
					break;
				}


				/********************************************************************************************
					16-bit Load Instructions
				*********************************************************************************************/

				// "LD reg value" B:3 C:12 FLAGS: - - - -
				case 0x01: instruction_ld16_reg_value(State.BC, (opcode[2] << 8) | (opcode[1])); State.PC += 2; m_cycles = 12; break;
				case 0x11: instruction_ld16_reg_value(State.DE, (opcode[2] << 8) | (opcode[1])); State.PC += 2; m_cycles = 12; break;
				case 0x21: instruction_ld16_reg_value(State.HL, (opcode[2] << 8) | (opcode[1])); State.PC += 2; m_cycles = 12; break;
				case 0x31: instruction_ld16_reg_value(State.SP, (opcode[2] << 8) | (opcode[1])); State.PC += 2; m_cycles = 12; break;

					// "LD [a16] SP" B:3 C:20 FLAGS: - - - -
				case 0x08:
				{
					uint16_t addr = (opcode[2] << 8) | (opcode[1]);
					gb.WriteToMemoryMap(addr, State.SP & 0x00FF);
					gb.WriteToMemoryMap(addr + 1, (State.SP & 0xFF00) >> 8);
					State.PC += 2;

					m_cycles = 20;
					break;
				}

				// "POP BC" B:1 C:12 FLAGS: - - - -
				case 0xC1:
				{
					State.BC = popSP();

					m_cycles = 12;
					break;
				}

				// "PUSH BC" B:1 C:16 FLAGS: - - - -
				case 0xC5:
				{
					pushSP(State.BC);

					m_cycles = 16;
					break;
				}

				// "POP DE" B:1 C:12 FLAGS: - - - -
				case 0xD1:
				{
					State.DE = popSP();

					m_cycles = 12;
					break;
				}

				// "PUSH DE" B:1 C:16 FLAGS: - - - -
				case 0xD5:
				{
					pushSP(State.DE);

					m_cycles = 16;
					break;
				}

				// "POP HL" B:1 C:12 FLAGS: - - - -
				case 0xE1:
				{
					State.HL = popSP();

					m_cycles = 12;
					break;
				}

				// "PUSH HL" B:1 C:16 FLAGS: - - - -
				case 0xE5:
				{
					pushSP(State.HL);

					m_cycles = 16;
					break;
				}

				// "POP AF" B:1 C:12 FLAGS: Z N H C
				case 0xF1:
				{
					State.AF = (popSP() & 0xFFF0);

					m_cycles = 12;
					break;
				}

				// "PUSH AF" B:1 C:16 FLAGS: - - - -
				case 0xF5:
				{
					pushSP(State.AF);

					m_cycles = 16;
					break;
				}

				// "LD HL SP e8" B:2 C:12 FLAGS: 0 0 H C
				case 0xF8:
				{
					int8_t signedValue = static_cast<int8_t>(opcode[1]);
					int32_t fullResult = State.SP + signedValue;
					uint16_t result = static_cast<uint16_t>(fullResult);

					SetCPUFlag(FLAG_ZERO, false);
					SetCPUFlag(FLAG_SUBTRACT, false);
					SetCPUFlag(FLAG_HALF_CARRY, ((State.SP ^ signedValue ^ (fullResult & 0xFFFF)) & 0x10) == 0x10);
					SetCPUFlag(FLAG_CARRY, ((State.SP ^ signedValue ^ (fullResult & 0xFFFF)) & 0x100) == 0x100);

					State.HL = result;
					State.PC++;

					m_cycles = 12;
					break;
				}

				// "LD SP HL" B:1 C:8 FLAGS: - - - -
				case 0xF9:
				{
					State.SP = State.HL;

					m_cycles = 8;
					break;
				}


				/********************************************************************************************
					8-bit Arithmetic/Logical Instructions
				*********************************************************************************************/

				// "INC reg" B:1 C:4 FLAGS: Z 0 H -
				case 0x04: instruction_inc_reg(State.B); m_cycles = 4; break;
				case 0x0C: instruction_inc_reg(State.C); m_cycles = 4; break;
				case 0x14: instruction_inc_reg(State.D); m_cycles = 4; break;
				case 0x1C: instruction_inc_reg(State.E); m_cycles = 4; break;
				case 0x24: instruction_inc_reg(State.H); m_cycles = 4; break;
				case 0x2C: instruction_inc_reg(State.L); m_cycles = 4; break;
				case 0x34: instruction_inc_hl(); m_cycles = 12; break;
				case 0x3C: instruction_inc_reg(State.A); m_cycles = 4; break;

					// "DEC reg" B:1 C:4 FLAGS: Z 1 H -
				case 0x05: instruction_dec_reg(State.B); m_cycles = 4; break;
				case 0x0D: instruction_dec_reg(State.C); m_cycles = 4; break;
				case 0x15: instruction_dec_reg(State.D); m_cycles = 4; break;
				case 0x1D: instruction_dec_reg(State.E); m_cycles = 4; break;
				case 0x25: instruction_dec_reg(State.H); m_cycles = 4; break;
				case 0x2D: instruction_dec_reg(State.L); m_cycles = 4; break;
				case 0x35: instruction_dec_hl(); m_cycles = 12; break;
				case 0x3D: instruction_dec_reg(State.A); m_cycles = 4; break;

					// "ADD A reg" B:1 C:4 FLAGS: Z 0 H C
				case 0x80: instruction_add_reg(State.B); m_cycles = 4; break;
				case 0x81: instruction_add_reg(State.C); m_cycles = 4; break;
				case 0x82: instruction_add_reg(State.D); m_cycles = 4; break;
				case 0x83: instruction_add_reg(State.E); m_cycles = 4; break;
				case 0x84: instruction_add_reg(State.H); m_cycles = 4; break;
				case 0x85: instruction_add_reg(State.L); m_cycles = 4; break;
				case 0x86: instruction_add_hl(); m_cycles = 8; break;
				case 0x87: instruction_add_reg(State.A); m_cycles = 4; break;
				case 0xC6: instruction_add_reg(opcode[1]); State.PC++; m_cycles = 8; break;

					// "ADC A reg" B:1 C:4 FLAGS: Z 0 H C
				case 0x88: instruction_adc_reg(State.B); m_cycles = 4; break;
				case 0x89: instruction_adc_reg(State.C); m_cycles = 4; break;
				case 0x8A: instruction_adc_reg(State.D); m_cycles = 4; break;
				case 0x8B: instruction_adc_reg(State.E); m_cycles = 4; break;
				case 0x8C: instruction_adc_reg(State.H); m_cycles = 4; break;
				case 0x8D: instruction_adc_reg(State.L); m_cycles = 4; break;
				case 0x8E: instruction_adc_hl(); m_cycles = 8; break;
				case 0x8F: instruction_adc_reg(State.A); m_cycles = 4; break;
				case 0xCE: instruction_adc_reg(opcode[1]); State.PC++; m_cycles = 8; break;

					// "SUB A reg" B:1 C:4 FLAGS: Z 1 H C
				case 0x90: instruction_sub_reg(State.B); m_cycles = 4; break;
				case 0x91: instruction_sub_reg(State.C); m_cycles = 4; break;
				case 0x92: instruction_sub_reg(State.D); m_cycles = 4; break;
				case 0x93: instruction_sub_reg(State.E); m_cycles = 4; break;
				case 0x94: instruction_sub_reg(State.H); m_cycles = 4; break;
				case 0x95: instruction_sub_reg(State.L); m_cycles = 4; break;
				case 0x96: instruction_sub_hl(); m_cycles = 8; break;
				case 0x97:
				{
					// SUB A A is a special case where the flags need to be 1 1 0 0
					instruction_sub_reg(State.A);
					SetCPUFlag(FLAG_ZERO, true);
					SetCPUFlag(FLAG_SUBTRACT, true);
					SetCPUFlag(FLAG_HALF_CARRY, false);
					SetCPUFlag(FLAG_CARRY, false);
					m_cycles = 4;
					break;
				}
				case 0xD6: instruction_sub_reg(opcode[1]); State.PC++; m_cycles = 8; break;

					// "SBC A reg" B:1 C:4 FLAGS: Z 1 H C
				case 0x98: instruction_sbc_reg(State.B); m_cycles = 4; break;
				case 0x99: instruction_sbc_reg(State.C); m_cycles = 4; break;
				case 0x9A: instruction_sbc_reg(State.D); m_cycles = 4; break;
				case 0x9B: instruction_sbc_reg(State.E); m_cycles = 4; break;
				case 0x9C: instruction_sbc_reg(State.H); m_cycles = 4; break;
				case 0x9D: instruction_sbc_reg(State.L); m_cycles = 4; break;
				case 0x9E: instruction_sbc_hl(); m_cycles = 8; break;
				case 0x9F: instruction_sbc_reg(State.A); m_cycles = 4; break;
				case 0xDE: instruction_sbc_reg(opcode[1]); State.PC++; m_cycles = 8; break;

					// "AND A reg" B:1 C:4 FLAGS: Z 0 1 0
				case 0xA0: instruction_and_reg(State.B); m_cycles = 4; break;
				case 0xA1: instruction_and_reg(State.C); m_cycles = 4; break;
				case 0xA2: instruction_and_reg(State.D); m_cycles = 4; break;
				case 0xA3: instruction_and_reg(State.E); m_cycles = 4; break;
				case 0xA4: instruction_and_reg(State.H); m_cycles = 4; break;
				case 0xA5: instruction_and_reg(State.L); m_cycles = 4; break;
				case 0xA6: instruction_and_hl(); m_cycles = 8; break;
				case 0xA7: instruction_and_reg(State.A); m_cycles = 4; break;
				case 0xE6: instruction_and_reg(opcode[1]); State.PC++; m_cycles = 8; break;

					// "XOR A reg" B:1 C:4 FLAGS: Z 0 0 0
				case 0xA8: instruction_xor_reg(State.B); m_cycles = 4; break;
				case 0xA9: instruction_xor_reg(State.C); m_cycles = 4; break;
				case 0xAA: instruction_xor_reg(State.D); m_cycles = 4; break;
				case 0xAB: instruction_xor_reg(State.E); m_cycles = 4; break;
				case 0xAC: instruction_xor_reg(State.H); m_cycles = 4; break;
				case 0xAD: instruction_xor_reg(State.L); m_cycles = 4; break;
				case 0xAE: instruction_xor_hl(); m_cycles = 8; break;
				case 0xAF: instruction_xor_reg(State.A); m_cycles = 4; break;
				case 0xEE: instruction_xor_reg(opcode[1]); State.PC++; m_cycles = 8; break;

					// "OR A reg" B:1 C:4 FLAGS: Z 0 0 0
				case 0xB0: instruction_or_reg(State.B); m_cycles = 4; break;
				case 0xB1: instruction_or_reg(State.C); m_cycles = 4; break;
				case 0xB2: instruction_or_reg(State.D); m_cycles = 4; break;
				case 0xB3: instruction_or_reg(State.E); m_cycles = 4; break;
				case 0xB4: instruction_or_reg(State.H); m_cycles = 4; break;
				case 0xB5: instruction_or_reg(State.L); m_cycles = 4; break;
				case 0xB6: instruction_or_hl(); m_cycles = 8; break;
				case 0xB7: instruction_or_reg(State.A); m_cycles = 4; break;
				case 0xF6: instruction_or_reg(opcode[1]); State.PC++; m_cycles = 8; break;

					// "CP A B" B:1 C:4 FLAGS: Z 1 H C
				case 0xB8: instruction_cp_reg(State.B); m_cycles = 4; break;
				case 0xB9: instruction_cp_reg(State.C); m_cycles = 4; break;
				case 0xBA: instruction_cp_reg(State.D); m_cycles = 4; break;
				case 0xBB: instruction_cp_reg(State.E); m_cycles = 4; break;
				case 0xBC: instruction_cp_reg(State.H); m_cycles = 4; break;
				case 0xBD: instruction_cp_reg(State.L); m_cycles = 4; break;
				case 0xBE: instruction_cp_hl(); m_cycles = 8; break;
				case 0xBF:
				{
					// CP A A is a special case where the flags need to be 1 1 0 0
					SetCPUFlag(FLAG_ZERO, true);
					SetCPUFlag(FLAG_SUBTRACT, true);
					SetCPUFlag(FLAG_HALF_CARRY, false);
					SetCPUFlag(FLAG_CARRY, false);

					m_cycles = 4;
					break;
				}
				case 0xFE: instruction_cp_reg(opcode[1]); State.PC++; m_cycles = 8; break;


					/********************************************************************************************
						16-bit Arithmetic/Logical Instructions
					*********************************************************************************************/

					// "INC reg16" B:1 C:8 FLAGS: - - - -
				case 0x03: instruction_inc_reg16(State.BC); m_cycles = 8; break;
				case 0x13: instruction_inc_reg16(State.DE); m_cycles = 8; break;
				case 0x23: instruction_inc_reg16(State.HL); m_cycles = 8; break;
				case 0x33: instruction_inc_reg16(State.SP); m_cycles = 8; break;

					// "DEC reg16" B:1 C:8 FLAGS: - - - -
				case 0x0B: instruction_dec_reg16(State.BC); m_cycles = 8; break;
				case 0x1B: instruction_dec_reg16(State.DE); m_cycles = 8; break;
				case 0x2B: instruction_dec_reg16(State.HL); m_cycles = 8; break;
				case 0x3B: instruction_dec_reg16(State.SP); m_cycles = 8; break;

					// "ADD HL reg16" B:1 C:8 FLAGS: - 0 H C
				case 0x09: instruction_add_reg16(State.BC); m_cycles = 8; break;
				case 0x19: instruction_add_reg16(State.DE); m_cycles = 8; break;
				case 0x29: instruction_add_reg16(State.HL); m_cycles = 8; break;
				case 0x39: instruction_add_reg16(State.SP); m_cycles = 8; break;

					// "ADD SP e8" B:2 C:16 FLAGS: 0 0 H C
				case 0xE8: instruction_add_sp_e8(opcode[1]); State.PC++; m_cycles = 16; break;

					/********************************************************************************************
						8-bit Shift. Rotate and Bit Instructions
					*********************************************************************************************/

					// "RLCA" B:1 C:4 FLAGS: 0 0 0 C
				case 0x07:
				{
					uint8_t result = (State.A << 1) | ((State.A >> 7) & 0x1);

					SetCPUFlag(FLAG_ZERO, false);
					SetCPUFlag(FLAG_SUBTRACT, false);
					SetCPUFlag(FLAG_HALF_CARRY, false);
					SetCPUFlag(FLAG_CARRY, ((State.A >> 7) & 0x1) != 0);

					State.A = result;

					m_cycles = 4;
					break;
				}

				// "RRCA" B:1 C:4 FLAGS: 0 0 0 C
				case 0x0F:
				{
					uint8_t result = ((State.A & 0x1) << 7) | (State.A >> 1);

					SetCPUFlag(FLAG_ZERO, false);
					SetCPUFlag(FLAG_SUBTRACT, false);
					SetCPUFlag(FLAG_HALF_CARRY, false);
					SetCPUFlag(FLAG_CARRY, (State.A & 0x1) != 0);

					State.A = result;

					m_cycles = 4;
					break;
				}

				// "RLA" B:1 C:4 FLAGS: 0 0 0 C
				case 0x17:
				{
					uint8_t result = (State.A << 1) | (GetCPUFlag(FLAG_CARRY) ? 1 : 0);
					bool carry = (State.A & 0x80) != 0;
					State.A = result;

					SetCPUFlag(FLAG_ZERO, false);
					SetCPUFlag(FLAG_SUBTRACT, false);
					SetCPUFlag(FLAG_HALF_CARRY, false);
					SetCPUFlag(FLAG_CARRY, carry);

					m_cycles = 4;
					break;
				}

				// "RRA" B:1 C:4 FLAGS: 0 0 0 C
				case 0x1F:
				{
					uint8_t carry = GetCPUFlag(FLAG_CARRY) ? 0x80 : 0;
					uint8_t result = carry | (State.A >> 1);

					SetCPUFlag(FLAG_ZERO, false);
					SetCPUFlag(FLAG_SUBTRACT, false);
					SetCPUFlag(FLAG_HALF_CARRY, false);
					SetCPUFlag(FLAG_CARRY, (State.A & 0x1) != 0);

					State.A = result;

					m_cycles = 4;
					break;
				}

				// "DAA" B:1 C:4 FLAGS: Z - 0 C
				case 0x27:
				{
					int32_t result = State.A;
					if (GetCPUFlag(FLAG_SUBTRACT))
					{
						if (GetCPUFlag(FLAG_HALF_CARRY))
						{
							result -= 6;
						}

						if (GetCPUFlag(FLAG_CARRY))
						{
							result -= 0x60;
						}
					}
					else
					{
						if (GetCPUFlag(FLAG_HALF_CARRY) || (State.A & 0x0F) > 9)
						{
							result += 6;
						}

						if (GetCPUFlag(FLAG_CARRY) || result > 0x9F)
						{
							result += 0x60;
						}
					}

					State.A = static_cast<uint8_t>(result);

					if (result > 0xFF)
						SetCPUFlag(FLAG_CARRY, true);

					SetCPUFlag(FLAG_ZERO, State.A == 0);
					SetCPUFlag(FLAG_HALF_CARRY, false);

					m_cycles = 4;
					break;
				}

				// "CPL" B:1 C:4 FLAGS: - 1 1 -
				case 0x2F:
				{
					State.A = ~State.A;
					SetCPUFlag(FLAG_SUBTRACT, true);
					SetCPUFlag(FLAG_HALF_CARRY, true);

					m_cycles = 4;
					break;
				}

				// "SCF" B:1 C:4 FLAGS: - 0 0 1
				case 0x37:
				{
					SetCPUFlag(FLAG_SUBTRACT, false);
					SetCPUFlag(FLAG_HALF_CARRY, false);
					SetCPUFlag(FLAG_CARRY, true);

					m_cycles = 4;
					break;
				}

				// "CCF" B:1 C:4 FLAGS: - 0 0 C
				case 0x3F:
				{
					// flip the carry flag
					GetCPUFlag(FLAG_CARRY) == true ? SetCPUFlag(FLAG_CARRY, false) : SetCPUFlag(FLAG_CARRY, true);

					SetCPUFlag(FLAG_SUBTRACT, false);
					SetCPUFlag(FLAG_HALF_CARRY, false);

					m_cycles = 4;
					break;
				}

				default:
					unimplementedInstruction(State, *opcode);
					break;
			}
		}

		processInterrupts();
		m_InstructionCompleted = true;
	}

	void Cpu::processInterrupts()
	{
		uint16_t destinationAddress = 0;
		uint8_t interruptFlag = 0;

		if (gb.ReadFromMemoryMapRegister(HW_INTERRUPT_ENABLE, IE_VBLANK) &&
			gb.ReadFromMemoryMapRegister(HW_IF_INTERRUPT_FLAG, IF_VBLANK))
		{
			destinationAddress = DEST_ADDRESS_VBLANK;
			interruptFlag = IF_VBLANK;
			m_isHalted = false;
		}

		if (gb.ReadFromMemoryMapRegister(HW_INTERRUPT_ENABLE, IE_LCD) &&
			gb.ReadFromMemoryMapRegister(HW_IF_INTERRUPT_FLAG, IF_LCD))
		{
			destinationAddress = DEST_ADDRESS_LCD_STAT;
			interruptFlag = IF_LCD;
			m_isHalted = false;
		}

		if (gb.ReadFromMemoryMapRegister(HW_INTERRUPT_ENABLE, IE_TIMER) &&
			gb.ReadFromMemoryMapRegister(HW_IF_INTERRUPT_FLAG, IF_TIMER))
		{
			destinationAddress = DEST_ADDRESS_TIMER;
			interruptFlag = IF_TIMER;
			m_isHalted = false;
		}

		if (gb.ReadFromMemoryMapRegister(HW_INTERRUPT_ENABLE, IE_SERIAL) &&
			gb.ReadFromMemoryMapRegister(HW_IF_INTERRUPT_FLAG, IF_SERIAL))
		{
			destinationAddress = DEST_ADDRESS_SERIAL;
			interruptFlag = IF_SERIAL;
			m_isHalted = false;
		}

		if (gb.ReadFromMemoryMapRegister(HW_INTERRUPT_ENABLE, IE_JOYPAD) &&
			gb.ReadFromMemoryMapRegister(HW_IF_INTERRUPT_FLAG, IF_JOYPAD))
		{
			destinationAddress = DEST_ADDRESS_JOYPAD;
			interruptFlag = IF_JOYPAD;
			m_isHalted = false;
		}

		if (destinationAddress != 0 && m_interruptMasterFlag)
		{
			m_interruptMasterFlag = false;;
			gb.WriteToMemoryMapRegister(HW_IF_INTERRUPT_FLAG, interruptFlag, false);

			pushSP(State.PC);
			State.PC = destinationAddress;
		}
	}

	void Cpu::process16bitInstruction(uint16_t opcode, Cpu::m_CpuState& state)
	{
		switch (opcode)
		{
			// Most instuctions take 8 cycles
			m_cycles = 8;

			// "RLC reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB00: instruction_rlc_reg(State.B); break;
			case 0xCB01: instruction_rlc_reg(State.C); break;
			case 0xCB02: instruction_rlc_reg(State.D); break;
			case 0xCB03: instruction_rlc_reg(State.E); break;
			case 0xCB04: instruction_rlc_reg(State.H); break;
			case 0xCB05: instruction_rlc_reg(State.L); break;
			case 0xCB06: instruction_rlc_hl(); m_cycles = 16; break;
			case 0xCB07: instruction_rlc_reg(State.A); break;

				// "RRC reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB08: instruction_rrc_reg(State.B); break;
			case 0xCB09: instruction_rrc_reg(State.C); break;
			case 0xCB0A: instruction_rrc_reg(State.D); break;
			case 0xCB0B: instruction_rrc_reg(State.E); break;
			case 0xCB0C: instruction_rrc_reg(State.H); break;
			case 0xCB0D: instruction_rrc_reg(State.L); break;
			case 0xCB0E: instruction_rrc_hl(); m_cycles = 16; break;
			case 0xCB0F: instruction_rrc_reg(State.A); break;

				// "RL reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB10: instruction_rl_reg(State.B); break;
			case 0xCB11: instruction_rl_reg(State.C); break;
			case 0xCB12: instruction_rl_reg(State.D); break;
			case 0xCB13: instruction_rl_reg(State.E); break;
			case 0xCB14: instruction_rl_reg(State.H); break;
			case 0xCB15: instruction_rl_reg(State.L); break;
			case 0xCB16: instruction_rl_hl(); m_cycles = 16; break;
			case 0xCB17: instruction_rl_reg(State.A); break;

				// "RR reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB18: instruction_rr_reg(State.B); break;
			case 0xCB19: instruction_rr_reg(State.C); break;
			case 0xCB1A: instruction_rr_reg(State.D); break;
			case 0xCB1B: instruction_rr_reg(State.E); break;
			case 0xCB1C: instruction_rr_reg(State.H); break;
			case 0xCB1D: instruction_rr_reg(State.L); break;
			case 0xCB1E: instruction_rr_hl(); m_cycles = 16; break;
			case 0xCB1F: instruction_rr_reg(State.A); break;

				// "SLA reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB20: instruction_sla_reg(State.B); break;
			case 0xCB21: instruction_sla_reg(State.C); break;
			case 0xCB22: instruction_sla_reg(State.D); break;
			case 0xCB23: instruction_sla_reg(State.E); break;
			case 0xCB24: instruction_sla_reg(State.H); break;
			case 0xCB25: instruction_sla_reg(State.L); break;
			case 0xCB26: instruction_sla_hl(); m_cycles = 16; break;
			case 0xCB27: instruction_sla_reg(State.A); break;

				// "SRA reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB28: instruction_sra_reg(State.B); break;
			case 0xCB29: instruction_sra_reg(State.C); break;
			case 0xCB2A: instruction_sra_reg(State.D); break;
			case 0xCB2B: instruction_sra_reg(State.E); break;
			case 0xCB2C: instruction_sra_reg(State.H); break;
			case 0xCB2D: instruction_sra_reg(State.L); break;
			case 0xCB2E: instruction_sra_hl(); m_cycles = 16; break;
			case 0xCB2F: instruction_sra_reg(State.A); break;

				// "SWAP reg" B:2 C:8 FLAGS: Z 0 0 0
			case 0xCB30: instruction_swap_reg(State.B); break;
			case 0xCB31: instruction_swap_reg(State.C); break;
			case 0xCB32: instruction_swap_reg(State.D); break;
			case 0xCB33: instruction_swap_reg(State.E); break;
			case 0xCB34: instruction_swap_reg(State.H); break;
			case 0xCB35: instruction_swap_reg(State.L); break;
			case 0xCB36: instruction_swap_hl(); m_cycles = 16; break;
			case 0xCB37: instruction_swap_reg(State.A); break;

				// "SRL bit" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB38: instruction_srl_reg(State.B); break;
			case 0xCB39: instruction_srl_reg(State.C); break;
			case 0xCB3A: instruction_srl_reg(State.D); break;
			case 0xCB3B: instruction_srl_reg(State.E); break;
			case 0xCB3C: instruction_srl_reg(State.H); break;
			case 0xCB3D: instruction_srl_reg(State.L); break;
			case 0xCB3E: instruction_srl_hl(); m_cycles = 16; break;
			case 0xCB3F: instruction_srl_reg(State.A); break;

				// "BIT bit, reg" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB40: instruction_bit_bit_reg(State.B, 0); break;
			case 0xCB41: instruction_bit_bit_reg(State.C, 0); break;
			case 0xCB42: instruction_bit_bit_reg(State.D, 0); break;
			case 0xCB43: instruction_bit_bit_reg(State.E, 0); break;
			case 0xCB44: instruction_bit_bit_reg(State.H, 0); break;
			case 0xCB45: instruction_bit_bit_reg(State.L, 0); break;
			case 0xCB46: instruction_bit_bit_hl(0); m_cycles = 12; break;
			case 0xCB47: instruction_bit_bit_reg(State.A, 0); break;
			case 0xCB48: instruction_bit_bit_reg(State.B, 1); break;
			case 0xCB49: instruction_bit_bit_reg(State.C, 1); break;
			case 0xCB4A: instruction_bit_bit_reg(State.D, 1); break;
			case 0xCB4B: instruction_bit_bit_reg(State.E, 1); break;
			case 0xCB4C: instruction_bit_bit_reg(State.H, 1); break;
			case 0xCB4D: instruction_bit_bit_reg(State.L, 1); break;
			case 0xCB4E: instruction_bit_bit_hl(1); m_cycles = 12; break;
			case 0xCB4F: instruction_bit_bit_reg(State.A, 1); break;
			case 0xCB50: instruction_bit_bit_reg(State.B, 2); break;
			case 0xCB51: instruction_bit_bit_reg(State.C, 2); break;
			case 0xCB52: instruction_bit_bit_reg(State.D, 2); break;
			case 0xCB53: instruction_bit_bit_reg(State.E, 2); break;
			case 0xCB54: instruction_bit_bit_reg(State.H, 2); break;
			case 0xCB55: instruction_bit_bit_reg(State.L, 2); break;
			case 0xCB56: instruction_bit_bit_hl(2); m_cycles = 12; break;
			case 0xCB57: instruction_bit_bit_reg(State.A, 2); break;
			case 0xCB58: instruction_bit_bit_reg(State.B, 3); break;
			case 0xCB59: instruction_bit_bit_reg(State.C, 3); break;
			case 0xCB5A: instruction_bit_bit_reg(State.D, 3); break;
			case 0xCB5B: instruction_bit_bit_reg(State.E, 3); break;
			case 0xCB5C: instruction_bit_bit_reg(State.H, 3); break;
			case 0xCB5D: instruction_bit_bit_reg(State.L, 3); break;
			case 0xCB5E: instruction_bit_bit_hl(3); m_cycles = 12; break;
			case 0xCB5F: instruction_bit_bit_reg(State.A, 3); break;
			case 0xCB60: instruction_bit_bit_reg(State.B, 4); break;
			case 0xCB61: instruction_bit_bit_reg(State.C, 4); break;
			case 0xCB62: instruction_bit_bit_reg(State.D, 4); break;
			case 0xCB63: instruction_bit_bit_reg(State.E, 4); break;
			case 0xCB64: instruction_bit_bit_reg(State.H, 4); break;
			case 0xCB65: instruction_bit_bit_reg(State.L, 4); break;
			case 0xCB66: instruction_bit_bit_hl(4); m_cycles = 12; break;
			case 0xCB67: instruction_bit_bit_reg(State.A, 4); break;
			case 0xCB68: instruction_bit_bit_reg(State.B, 5); break;
			case 0xCB69: instruction_bit_bit_reg(State.C, 5); break;
			case 0xCB6A: instruction_bit_bit_reg(State.D, 5); break;
			case 0xCB6B: instruction_bit_bit_reg(State.E, 5); break;
			case 0xCB6C: instruction_bit_bit_reg(State.H, 5); break;
			case 0xCB6D: instruction_bit_bit_reg(State.L, 5); break;
			case 0xCB6E: instruction_bit_bit_hl(5); m_cycles = 12; break;
			case 0xCB6F: instruction_bit_bit_reg(State.A, 5); break;
			case 0xCB70: instruction_bit_bit_reg(State.B, 6); break;
			case 0xCB71: instruction_bit_bit_reg(State.C, 6); break;
			case 0xCB72: instruction_bit_bit_reg(State.D, 6); break;
			case 0xCB73: instruction_bit_bit_reg(State.E, 6); break;
			case 0xCB74: instruction_bit_bit_reg(State.H, 6); break;
			case 0xCB75: instruction_bit_bit_reg(State.L, 6); break;
			case 0xCB76: instruction_bit_bit_hl(6); m_cycles = 12; break;
			case 0xCB77: instruction_bit_bit_reg(State.A, 6); break;
			case 0xCB78: instruction_bit_bit_reg(State.B, 7); break;
			case 0xCB79: instruction_bit_bit_reg(State.C, 7); break;
			case 0xCB7A: instruction_bit_bit_reg(State.D, 7); break;
			case 0xCB7B: instruction_bit_bit_reg(State.E, 7); break;
			case 0xCB7C: instruction_bit_bit_reg(State.H, 7); break;
			case 0xCB7D: instruction_bit_bit_reg(State.L, 7); break;
			case 0xCB7E: instruction_bit_bit_hl(7); m_cycles = 12; break;
			case 0xCB7F: instruction_bit_bit_reg(State.A, 7); break;

				// "RES bit, reg" B:2 C:8 FLAGS: - - - -
			case 0xCB80: instruction_res_bit_reg(State.B, 0); break;
			case 0xCB81: instruction_res_bit_reg(State.C, 0); break;
			case 0xCB82: instruction_res_bit_reg(State.D, 0); break;
			case 0xCB83: instruction_res_bit_reg(State.E, 0); break;
			case 0xCB84: instruction_res_bit_reg(State.H, 0); break;
			case 0xCB85: instruction_res_bit_reg(State.L, 0); break;
			case 0xCB86: instruction_res_bit_hl(0); m_cycles = 16; break;
			case 0xCB87: instruction_res_bit_reg(State.A, 0); break;
			case 0xCB88: instruction_res_bit_reg(State.B, 1); break;
			case 0xCB89: instruction_res_bit_reg(State.C, 1); break;
			case 0xCB8A: instruction_res_bit_reg(State.D, 1); break;
			case 0xCB8B: instruction_res_bit_reg(State.E, 1); break;
			case 0xCB8C: instruction_res_bit_reg(State.H, 1); break;
			case 0xCB8D: instruction_res_bit_reg(State.L, 1); break;
			case 0xCB8E: instruction_res_bit_hl(1); m_cycles = 16; break;
			case 0xCB8F: instruction_res_bit_reg(State.A, 1); break;
			case 0xCB90: instruction_res_bit_reg(State.B, 2); break;
			case 0xCB91: instruction_res_bit_reg(State.C, 2); break;
			case 0xCB92: instruction_res_bit_reg(State.D, 2); break;
			case 0xCB93: instruction_res_bit_reg(State.E, 2); break;
			case 0xCB94: instruction_res_bit_reg(State.H, 2); break;
			case 0xCB95: instruction_res_bit_reg(State.L, 2); break;
			case 0xCB96: instruction_res_bit_hl(2); m_cycles = 16; break;
			case 0xCB97: instruction_res_bit_reg(State.A, 2); break;
			case 0xCB98: instruction_res_bit_reg(State.B, 3); break;
			case 0xCB99: instruction_res_bit_reg(State.C, 3); break;
			case 0xCB9A: instruction_res_bit_reg(State.D, 3); break;
			case 0xCB9B: instruction_res_bit_reg(State.E, 3); break;
			case 0xCB9C: instruction_res_bit_reg(State.H, 3); break;
			case 0xCB9D: instruction_res_bit_reg(State.L, 3); break;
			case 0xCB9E: instruction_res_bit_hl(3); m_cycles = 16; break;
			case 0xCB9F: instruction_res_bit_reg(State.A, 3); break;
			case 0xCBA0: instruction_res_bit_reg(State.B, 4); break;
			case 0xCBA1: instruction_res_bit_reg(State.C, 4); break;
			case 0xCBA2: instruction_res_bit_reg(State.D, 4); break;
			case 0xCBA3: instruction_res_bit_reg(State.E, 4); break;
			case 0xCBA4: instruction_res_bit_reg(State.H, 4); break;
			case 0xCBA5: instruction_res_bit_reg(State.L, 4); break;
			case 0xCBA6: instruction_res_bit_hl(4); m_cycles = 16; break;
			case 0xCBA7: instruction_res_bit_reg(State.A, 4); break;
			case 0xCBA8: instruction_res_bit_reg(State.B, 5); break;
			case 0xCBA9: instruction_res_bit_reg(State.C, 5); break;
			case 0xCBAA: instruction_res_bit_reg(State.D, 5); break;
			case 0xCBAB: instruction_res_bit_reg(State.E, 5); break;
			case 0xCBAC: instruction_res_bit_reg(State.H, 5); break;
			case 0xCBAD: instruction_res_bit_reg(State.L, 5); break;
			case 0xCBAE: instruction_res_bit_hl(5); m_cycles = 16; break;
			case 0xCBAF: instruction_res_bit_reg(State.A, 5); break;
			case 0xCBB0: instruction_res_bit_reg(State.B, 6); break;
			case 0xCBB1: instruction_res_bit_reg(State.C, 6); break;
			case 0xCBB2: instruction_res_bit_reg(State.D, 6); break;
			case 0xCBB3: instruction_res_bit_reg(State.E, 6); break;
			case 0xCBB4: instruction_res_bit_reg(State.H, 6); break;
			case 0xCBB5: instruction_res_bit_reg(State.L, 6); break;
			case 0xCBB6: instruction_res_bit_hl(6); m_cycles = 16; break;
			case 0xCBB7: instruction_res_bit_reg(State.A, 6); break;
			case 0xCBB8: instruction_res_bit_reg(State.B, 7); break;
			case 0xCBB9: instruction_res_bit_reg(State.C, 7); break;
			case 0xCBBA: instruction_res_bit_reg(State.D, 7); break;
			case 0xCBBB: instruction_res_bit_reg(State.E, 7); break;
			case 0xCBBC: instruction_res_bit_reg(State.H, 7); break;
			case 0xCBBD: instruction_res_bit_reg(State.L, 7); break;
			case 0xCBBE: instruction_res_bit_hl(7); m_cycles = 16; break;
			case 0xCBBF: instruction_res_bit_reg(State.A, 7); break;

				// "SET bit, register" B:2 C:8 - - - -
			case 0xCBC0: instruction_set_bit_reg(State.B, 0); break;
			case 0xCBC1: instruction_set_bit_reg(State.C, 0); break;
			case 0xCBC2: instruction_set_bit_reg(State.D, 0); break;
			case 0xCBC3: instruction_set_bit_reg(State.E, 0); break;
			case 0xCBC4: instruction_set_bit_reg(State.H, 0); break;
			case 0xCBC5: instruction_set_bit_reg(State.L, 0); break;
			case 0xCBC6: instruction_set_bit_hl(0); m_cycles = 16; break;
			case 0xCBC7: instruction_set_bit_reg(State.A, 0); break;
			case 0xCBC8: instruction_set_bit_reg(State.B, 1); break;
			case 0xCBC9: instruction_set_bit_reg(State.C, 1); break;
			case 0xCBCA: instruction_set_bit_reg(State.D, 1); break;
			case 0xCBCB: instruction_set_bit_reg(State.E, 1); break;
			case 0xCBCC: instruction_set_bit_reg(State.H, 1); break;
			case 0xCBCD: instruction_set_bit_reg(State.L, 1); break;
			case 0xCBCE: instruction_set_bit_hl(1); m_cycles = 16; break;
			case 0xCBCF: instruction_set_bit_reg(State.A, 1); break;
			case 0xCBD0: instruction_set_bit_reg(State.B, 2); break;
			case 0xCBD1: instruction_set_bit_reg(State.C, 2); break;
			case 0xCBD2: instruction_set_bit_reg(State.D, 2); break;
			case 0xCBD3: instruction_set_bit_reg(State.E, 2); break;
			case 0xCBD4: instruction_set_bit_reg(State.H, 2); break;
			case 0xCBD5: instruction_set_bit_reg(State.L, 2); break;
			case 0xCBD6: instruction_set_bit_hl(2); m_cycles = 16; break;
			case 0xCBD7: instruction_set_bit_reg(State.A, 2); break;
			case 0xCBD8: instruction_set_bit_reg(State.B, 3); break;
			case 0xCBD9: instruction_set_bit_reg(State.C, 3); break;
			case 0xCBDA: instruction_set_bit_reg(State.D, 3); break;
			case 0xCBDB: instruction_set_bit_reg(State.E, 3); break;
			case 0xCBDC: instruction_set_bit_reg(State.H, 3); break;
			case 0xCBDD: instruction_set_bit_reg(State.L, 3); break;
			case 0xCBDE: instruction_set_bit_hl(3); m_cycles = 16; break;
			case 0xCBDF: instruction_set_bit_reg(State.A, 3); break;
			case 0xCBE0: instruction_set_bit_reg(State.B, 4); break;
			case 0xCBE1: instruction_set_bit_reg(State.C, 4); break;
			case 0xCBE2: instruction_set_bit_reg(State.D, 4); break;
			case 0xCBE3: instruction_set_bit_reg(State.E, 4); break;
			case 0xCBE4: instruction_set_bit_reg(State.H, 4); break;
			case 0xCBE5: instruction_set_bit_reg(State.L, 4); break;
			case 0xCBE6: instruction_set_bit_hl(4); m_cycles = 16; break;
			case 0xCBE7: instruction_set_bit_reg(State.A, 4); break;
			case 0xCBE8: instruction_set_bit_reg(State.B, 5); break;
			case 0xCBE9: instruction_set_bit_reg(State.C, 5); break;
			case 0xCBEA: instruction_set_bit_reg(State.D, 5); break;
			case 0xCBEB: instruction_set_bit_reg(State.E, 5); break;
			case 0xCBEC: instruction_set_bit_reg(State.H, 5); break;
			case 0xCBED: instruction_set_bit_reg(State.L, 5); break;
			case 0xCBEE: instruction_set_bit_hl(5); m_cycles = 16; break;
			case 0xCBEF: instruction_set_bit_reg(State.A, 5); break;
			case 0xCBF0: instruction_set_bit_reg(State.B, 6); break;
			case 0xCBF1: instruction_set_bit_reg(State.C, 6); break;
			case 0xCBF2: instruction_set_bit_reg(State.D, 6); break;
			case 0xCBF3: instruction_set_bit_reg(State.E, 6); break;
			case 0xCBF4: instruction_set_bit_reg(State.H, 6); break;
			case 0xCBF5: instruction_set_bit_reg(State.L, 6); break;
			case 0xCBF6: instruction_set_bit_hl(6); m_cycles = 16; break;
			case 0xCBF7: instruction_set_bit_reg(State.A, 6); break;
			case 0xCBF8: instruction_set_bit_reg(State.B, 7); break;
			case 0xCBF9: instruction_set_bit_reg(State.C, 7); break;
			case 0xCBFA: instruction_set_bit_reg(State.D, 7); break;
			case 0xCBFB: instruction_set_bit_reg(State.E, 7); break;
			case 0xCBFC: instruction_set_bit_reg(State.H, 7); break;
			case 0xCBFD: instruction_set_bit_reg(State.L, 7); break;
			case 0xCBFE: instruction_set_bit_hl(7); m_cycles = 16; break;
			case 0xCBFF: instruction_set_bit_reg(State.A, 7); break;

			default:
				unimplementedInstruction(State, opcode);
				break;
		}
	}

	void Cpu::unimplementedInstruction(Cpu::m_CpuState& State, uint8_t opcode)
	{
		Logger::Instance().LogMessage(LogMessageType::MMU, "Error: Unimplemented instruction: %02x"/*, opcode */);
	}

	int Cpu::Disassemble(uint8_t* opcode, int pc)
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

	std::map<uint16_t, std::string> Cpu::DisassebleAll()
	{
		std::map<uint16_t, std::string> mapLines;
		uint16_t pc = 0;

		while (pc < 0x7FFF)
		{
			uint8_t* opcode = &gb.ReadFromMemoryMap(pc);
			int nextPC = Disassemble(opcode, pc);
			mapLines[pc] = GetCurrentInstruction();
			pc += nextPC;
		}

		return mapLines;
	}

	void Cpu::disasseble16bit(uint8_t* opcode, int pc)
	{
		// Note: All 16 bit opcodes are 2 bytes.
		uint16_t opcode16bit = (opcode[0] << 8) | opcode[1];

		switch (opcode16bit)
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

	void Cpu::outputDisassembledInstruction(const char* instructionName, int pc, uint8_t* opcode, int totalOpBytes)
	{
		/* Ideal format
			0000 00       NOP
			0001 c3 d4 18 JMP    $18d4
		*/

		std::string output;

		// print program counter address
		output += FormatHex(pc, 4) + " ";

		// print flags
		//output += "Z" + FormatHex(GetCPUFlag(FLAG_ZERO), 1) + " ";
		//output += "N" + FormatHex(GetCPUFlag(FLAG_SUBTRACT), 1) + " ";
		//output += "H" + FormatHex(GetCPUFlag(FLAG_HALF_CARRY), 1) + " ";
		//output += "C" + FormatHex(GetCPUFlag(FLAG_CARRY), 1) + " ";

		// print address values
		if (totalOpBytes == 3)
		{
			output += FormatHex(opcode[0], 2) + " ";
			output += FormatHex(opcode[1], 2) + " ";
			output += FormatHex(opcode[2], 2) + " ";
		}
		else if (totalOpBytes == 2)
		{
			output += FormatHex(opcode[0], 2) + " ";
			output += FormatHex(opcode[1], 2) + " ";
			output += "   ";
		}
		else
		{
			output += FormatHex(opcode[0], 2) + " ";
			output += "   ";
			output += "   ";
		}

		// print instruction name
		std::string i(instructionName);
		output += " " + i + " ";

		// print address
		if (totalOpBytes == 3)
		{
			output += "$";
			output += FormatHex(opcode[2], 2);
			output += FormatHex(opcode[1], 2);
		}
		else if (totalOpBytes == 2)
		{
			output += "$0x";
			output += FormatHex(opcode[1], 2);
		}

		// new line
		output += "\n";

		currentInstructionName = output;
		Logger::Instance().LogCPUMessage(output);
	}

	std::string Cpu::GetCurrentInstruction()
	{
		return currentInstructionName;
	}

	void Cpu::Reset(bool enableBootRom)
	{
		Cpu::m_TotalCycles = 0;

		// registers
		State.AF = enableBootRom ? 0x000 : 0x01B0;
		State.BC = enableBootRom ? 0x000 : 0x0013;
		State.DE = enableBootRom ? 0x000 : 0x00D8;
		State.HL = enableBootRom ? 0x000 : 0x014D;
		State.PC = enableBootRom ? 0x000 : 0x100; // game boy execution start point
		State.SP = enableBootRom ? 0x000 : 0xFFFE;

		// flags - should be reset to $B0
		SetCPUFlag(FLAG_CARRY, true);
		SetCPUFlag(FLAG_HALF_CARRY, true);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_ZERO, true);

		// randomize memory to mimick real gameboy
		for (uint16_t address = 0x8000; address <= 0x97FF; address++)
		{
			gb.WriteToMemoryMap(address, generateRandomNumber(0, 255));
		}

		// hardware registers
		gb.WriteToMemoryMap(0xFF00, 0xCF);
		gb.WriteToMemoryMap(0xFF01, 0x00);
		gb.WriteToMemoryMap(0xFF02, 0x7E);
		gb.WriteToMemoryMap(0xFF04, 0xAB);
		gb.WriteToMemoryMap(0xFF05, 0x00);
		gb.WriteToMemoryMap(0xFF06, 0x00);
		gb.WriteToMemoryMap(0xFF07, 0xF8);
		gb.WriteToMemoryMap(0xFF0F, 0xE1);
		gb.WriteToMemoryMap(0xFF10, 0x80);
		gb.WriteToMemoryMap(0xFF11, 0xBF);
		gb.WriteToMemoryMap(0xFF12, 0xF3);
		gb.WriteToMemoryMap(0xFF13, 0xFF);
		gb.WriteToMemoryMap(0xFF14, 0xBF);
		gb.WriteToMemoryMap(0xFF16, 0x3F);
		gb.WriteToMemoryMap(0xFF17, 0x00);
		gb.WriteToMemoryMap(0xFF18, 0xFF);
		gb.WriteToMemoryMap(0xFF19, 0xBF);
		gb.WriteToMemoryMap(0xFF1A, 0x7F);
		gb.WriteToMemoryMap(0xFF1B, 0xFF);
		gb.WriteToMemoryMap(0xFF1C, 0x9F);
		gb.WriteToMemoryMap(0xFF1D, 0xFF);
		gb.WriteToMemoryMap(0xFF1E, 0xBF);
		gb.WriteToMemoryMap(0xFF20, 0xFF);
		gb.WriteToMemoryMap(0xFF21, 0x00);
		gb.WriteToMemoryMap(0xFF22, 0x00);
		gb.WriteToMemoryMap(0xFF23, 0xBF);
		gb.WriteToMemoryMap(0xFF24, 0x77);
		gb.WriteToMemoryMap(0xFF25, 0xF3);
		gb.WriteToMemoryMap(0xFF26, 0xF1);
		gb.WriteToMemoryMap(0xFF40, 0x91);
		gb.WriteToMemoryMap(0xFF41, 0x85);
		gb.WriteToMemoryMap(0xFF42, 0x00);
		gb.WriteToMemoryMap(0xFF43, 0x00);
		gb.WriteToMemoryMap(0xFF44, 0x00);
		gb.WriteToMemoryMap(0xFF45, 0x00);
		gb.WriteToMemoryMap(0xFF46, 0xFF);
		gb.WriteToMemoryMap(0xFF47, 0xFC);
		gb.WriteToMemoryMap(0xFF48, 0x00);
		gb.WriteToMemoryMap(0xFF49, 0x00);
		gb.WriteToMemoryMap(0xFF4A, 0x00);
		gb.WriteToMemoryMap(0xFF4B, 0x00);
		gb.WriteToMemoryMap(0xFF4D, 0xFF);
		gb.WriteToMemoryMap(0xFF4F, 0xFF);
		gb.WriteToMemoryMap(0xFF51, 0xFF);
		gb.WriteToMemoryMap(0xFF52, 0xFF);
		gb.WriteToMemoryMap(0xFF53, 0xFF);
		gb.WriteToMemoryMap(0xFF54, 0xFF);
		gb.WriteToMemoryMap(0xFF55, 0xFF);
		gb.WriteToMemoryMap(0xFF56, 0xFF);
		gb.WriteToMemoryMap(0xFF68, 0xFF);
		gb.WriteToMemoryMap(0xFF69, 0xFF);
		gb.WriteToMemoryMap(0xFF6A, 0xFF);
		gb.WriteToMemoryMap(0xFF6B, 0xFF);
		gb.WriteToMemoryMap(0xFF70, 0xFF);
		gb.WriteToMemoryMap(HW_INTERRUPT_ENABLE, 0x00);
	}

	void Cpu::pushSP(uint16_t value)
	{
		gb.WriteToMemoryMap(--State.SP, (value >> 8) & 0xFF);
		gb.WriteToMemoryMap(--State.SP, value & 0xFF);
	}

	uint16_t Cpu::popSP()
	{
		uint8_t firstByte = gb.ReadFromMemoryMap(State.SP++);
		uint8_t secondByte = gb.ReadFromMemoryMap(State.SP++);
		return (secondByte << 8) | (firstByte);
	}

	bool Cpu::GetCPUFlag(int flag)
	{
		return (State.F & flag) != 0;
	}

	void Cpu::SetCPUFlag(int flag, bool enable)
	{
		if (enable)
			State.F |= flag;
		else
			State.F &= ~flag;
	}

	void Cpu::instruction_ld_reg_value(uint8_t& reg, uint8_t& value)
	{
		reg = value;
	}

	void Cpu::instruction_ld_addr_reg(uint16_t& address, uint8_t& reg)
	{
		gb.WriteToMemoryMap(address, reg);
	}

	void Cpu::instruction_ld_reg_addr(uint8_t& reg, uint16_t& address)
	{
		reg = gb.ReadFromMemoryMap(address);
	}

	void Cpu::instruction_ld16_reg_value(uint16_t& reg, uint16_t value)
	{
		reg = value;
	}

	void Cpu::instruction_inc_reg(uint8_t& reg)
	{
		reg++;

		SetCPUFlag(FLAG_ZERO, (reg == 0));
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, (reg & 0x0F) == 0x00);
	}

	void Cpu::instruction_inc_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_inc_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_dec_reg(uint8_t& reg)
	{
		reg--;

		SetCPUFlag(FLAG_ZERO, (reg == 0));
		SetCPUFlag(FLAG_SUBTRACT, true);
		SetCPUFlag(FLAG_HALF_CARRY, (reg & 0x0F) == 0x0F);
	}

	void Cpu::instruction_dec_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_dec_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_add_reg(uint8_t& reg)
	{
		uint16_t fullResult = State.A + reg;
		uint8_t result = static_cast<uint8_t>(fullResult);

		// Update flags
		SetCPUFlag(FLAG_ZERO, result == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, ((State.A & 0x0F) + (reg & 0x0F) > 0x0F));
		SetCPUFlag(FLAG_CARRY, (fullResult > 0xFF));

		State.A = result;
	}

	void Cpu::instruction_add_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_add_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_adc_reg(uint8_t& reg)
	{
		bool carry = GetCPUFlag(FLAG_CARRY);
		uint16_t fullResult = State.A + reg + (carry ? 1 : 0);
		uint8_t result = static_cast<uint8_t>(fullResult);

		// Update flags
		SetCPUFlag(FLAG_ZERO, result == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, (State.A & 0xF) + (reg & 0xF) + (carry ? 1 : 0) > 0xF);
		SetCPUFlag(FLAG_CARRY, (fullResult > 0xFF));

		State.A = result;
	}

	void Cpu::instruction_adc_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_adc_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_sub_reg(uint8_t& reg)
	{
		uint8_t result = State.A - reg;

		// Update flags
		SetCPUFlag(FLAG_ZERO, result == 0);
		SetCPUFlag(FLAG_SUBTRACT, true);
		SetCPUFlag(FLAG_HALF_CARRY, (State.A & 0x0F) - (reg & 0x0F) < 0);
		SetCPUFlag(FLAG_CARRY, State.A < reg);

		State.A = result;
	}

	void Cpu::instruction_sub_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_sub_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_sbc_reg(uint8_t& reg)
	{
		bool carry = GetCPUFlag(FLAG_CARRY);
		int32_t fullResult = State.A - reg - (carry ? 1 : 0);
		uint8_t result = static_cast<uint8_t>(fullResult);

		SetCPUFlag(FLAG_ZERO, result == 0);
		SetCPUFlag(FLAG_SUBTRACT, true);
		SetCPUFlag(FLAG_HALF_CARRY, (State.A & 0xF) - (reg & 0xF) - carry < 0);
		SetCPUFlag(FLAG_CARRY, fullResult < 0);

		State.A = result;
	}

	void Cpu::instruction_sbc_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_sbc_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_and_reg(uint8_t& reg)
	{
		State.A = State.A & reg;

		SetCPUFlag(FLAG_ZERO, State.A == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, true);
		SetCPUFlag(FLAG_CARRY, false);
	}

	void Cpu::instruction_and_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_and_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_xor_reg(uint8_t& reg)
	{
		State.A = State.A ^ reg;

		SetCPUFlag(FLAG_ZERO, State.A == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
		SetCPUFlag(FLAG_CARRY, false);
	}

	void Cpu::instruction_xor_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_xor_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_or_reg(uint8_t& reg)
	{
		State.A = State.A | reg;

		SetCPUFlag(FLAG_ZERO, State.A == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
		SetCPUFlag(FLAG_CARRY, false);
	}

	void Cpu::instruction_or_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_or_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_cp_reg(uint8_t& reg)
	{
		SetCPUFlag(FLAG_ZERO, State.A == reg);
		SetCPUFlag(FLAG_SUBTRACT, true);
		SetCPUFlag(FLAG_HALF_CARRY, (State.A & 0xF) - (reg & 0xF) < 0);
		SetCPUFlag(FLAG_CARRY, State.A < reg);
	}

	void Cpu::instruction_cp_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_cp_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_inc_reg16(uint16_t& reg)
	{
		reg++;
	}

	void Cpu::instruction_dec_reg16(uint16_t& reg)
	{
		reg--;
	}

	void Cpu::instruction_add_reg16(uint16_t& reg)
	{
		uint32_t fullResult = State.HL + reg;
		uint16_t result = static_cast<uint16_t>(fullResult);

		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, (State.HL ^ reg ^ (fullResult & 0xFFFF)) & 0x1000);
		SetCPUFlag(FLAG_CARRY, fullResult > 0xFFFF);

		State.HL = result;
	}

	void Cpu::instruction_add_sp_e8(uint8_t& e8)
	{
		int8_t offset = e8;
		int32_t fullResult = State.SP + offset;
		uint16_t result = static_cast<uint16_t>(fullResult);

		SetCPUFlag(FLAG_ZERO, false);
		SetCPUFlag(FLAG_SUBTRACT, false);

		SetCPUFlag(FLAG_HALF_CARRY, ((State.SP ^ offset ^ (fullResult & 0xFFFF)) & 0x10) == 0x10);
		SetCPUFlag(FLAG_CARRY, ((State.SP ^ offset ^ (fullResult & 0xFFFF)) & 0x100) == 0x100);

		State.SP = result;
	}

	void Cpu::instruction_rlc_reg(uint8_t& reg)
	{
		uint8_t result = (reg << 1) | ((reg >> 7) & 0x1);

		SetCPUFlag(FLAG_ZERO, result == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
		SetCPUFlag(FLAG_CARRY, ((reg >> 7) & 0x1) != 0);

		reg = result;
	}

	void Cpu::instruction_rlc_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_rlc_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_rrc_reg(uint8_t& reg)
	{
		SetCPUFlag(FLAG_CARRY, (reg & 0x01) != 0);

		reg = (reg >> 1) | (GetCPUFlag(FLAG_CARRY) ? 0x80 : 0x00);

		SetCPUFlag(FLAG_ZERO, (reg == 0));
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
	}

	void Cpu::instruction_rrc_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_rrc_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_rl_reg(uint8_t& reg)
	{
		// Rotate left through carry
		bool carry = GetCPUFlag(FLAG_CARRY);
		uint8_t temp = (reg << 1) | (carry ? 1 : 0);
		carry = (reg & 0x80) != 0;
		reg = temp;

		// Update flags
		SetCPUFlag(FLAG_ZERO, (reg == 0));
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
		SetCPUFlag(FLAG_CARRY, carry);
	}

	void Cpu::instruction_rl_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_rl_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_rr_reg(uint8_t& reg)
	{
		uint8_t carry = GetCPUFlag(FLAG_CARRY) ? 0x80 : 0;
		uint8_t result = carry | (reg >> 1);

		SetCPUFlag(FLAG_ZERO, result == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
		SetCPUFlag(FLAG_CARRY, (reg & 0x1) != 0);

		reg = result;
	}

	void Cpu::instruction_rr_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_rr_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_sla_reg(uint8_t& reg)
	{
		SetCPUFlag(FLAG_CARRY, (reg & 0x80) != 0);

		reg = reg << 1;

		SetCPUFlag(FLAG_ZERO, (reg == 0));
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
	}

	void Cpu::instruction_sla_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_sla_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_sra_reg(uint8_t& reg)
	{
		uint8_t result = (reg & 0x80) | (reg >> 1);

		SetCPUFlag(FLAG_ZERO, result == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
		SetCPUFlag(FLAG_CARRY, ((reg & 0x1) != 0));

		reg = result;
	}

	void Cpu::instruction_sra_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_sra_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_swap_reg(uint8_t& reg)
	{
		reg = ((reg & 0x0F) << 4) | ((reg & 0xF0) >> 4);

		SetCPUFlag(FLAG_ZERO, (reg == 0));
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
		SetCPUFlag(FLAG_CARRY, false);
	}

	void Cpu::instruction_swap_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_swap_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_srl_reg(uint8_t& reg)
	{
		SetCPUFlag(FLAG_CARRY, (reg & 0x01) != 0);

		reg = reg >> 1;

		SetCPUFlag(FLAG_ZERO, (reg == 0));
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
	}

	void Cpu::instruction_srl_hl()
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_srl_reg(value);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_bit_bit_reg(uint8_t& reg, uint8_t bit)
	{
		SetCPUFlag(FLAG_ZERO, (reg & (1 << bit)) == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, true);
	}

	void Cpu::instruction_bit_bit_hl(uint8_t bit)
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_bit_bit_reg(value, bit);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_res_bit_reg(uint8_t& reg, uint8_t bit)
	{
		reg &= ~(1 << bit);
	}

	void Cpu::instruction_res_bit_hl(uint8_t bit)
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_res_bit_reg(value, bit);
		gb.WriteToMemoryMap(State.HL, value);
	}

	void Cpu::instruction_set_bit_reg(uint8_t& reg, uint8_t bit)
	{
		reg |= (1 << bit);
	}

	void Cpu::instruction_set_bit_hl(uint8_t bit)
	{
		uint8_t value = gb.ReadFromMemoryMap(State.HL);
		instruction_set_bit_reg(value, bit);
		gb.WriteToMemoryMap(State.HL, value);
	}

	int Cpu::getClockSelect()
	{
		bool lowBit = gb.ReadFromMemoryMapRegister(HW_TAC_TIMER_CONTROL, TimerControl_Flags::TAC_CLOCK_SELECT_LBIT);
		bool highBit = gb.ReadFromMemoryMapRegister(HW_TAC_TIMER_CONTROL, TimerControl_Flags::TAC_CLOCK_SELECT_HBIT);

		uint8_t value = 0;
		value |= lowBit ? 0x01 : 0;
		value |= highBit ? 0x02 : 0;

		if (value == 0x00)
			return TIMA_CLOCK_SPEED_00;
		if (value == 0x01)
			return TIMA_CLOCK_SPEED_01;
		if (value == 0x10)
			return TIMA_CLOCK_SPEED_10;
		if (value == 0x11)
			return TIMA_CLOCK_SPEED_11;
	}

	void Cpu::processTimers() 
	{
		// increment DIV register
		uint16_t internalClock = (gb.ReadFromMemoryMap(HW_DIV_DIVIDER_REGISTER) << 8)
			| gb.ReadFromMemoryMap(HW_DIV_DIVIDER_REGISTER_LOW);

		internalClock++;

		// if the div clock rolls over then we need to copy the value of TIMA to TMA
		if (internalClock == 0xFFFF)
		{
			gb.WriteToMemoryMap(HW_TMA_TIMER_MODULO, gb.ReadFromMemoryMap(HW_TIMA_TIMER_COUNTER));
		}

		// write updated DIV register
		gb.WriteToMemoryMap(HW_DIV_DIVIDER_REGISTER, (internalClock & 0xFF00) >> 8);
		gb.WriteToMemoryMap(HW_DIV_DIVIDER_REGISTER_LOW, internalClock & 0x00FF);

		// now increment the TIMA register
		uint8_t tima = gb.ReadFromMemoryMap(HW_TIMA_TIMER_COUNTER);

		// the TIMA register ticks at a specific clock speed based on the TAC Timer Control register
		if (internalClock % getClockSelect() == 0)
		{
			tima++;
			gb.WriteToMemoryMap(HW_TIMA_TIMER_COUNTER, tima);
		}

		// if the TIMA register rolls over then we need to trigger an interrupt
		if (tima == 0xFF)
		{
			gb.WriteToMemoryMap(HW_TIMA_TIMER_COUNTER, gb.ReadFromMemoryMap(HW_TMA_TIMER_MODULO));

			if (gb.ReadFromMemoryMapRegister(HW_TAC_TIMER_CONTROL, TAC_ENABLE))
			{
				gb.WriteToMemoryMapRegister(HW_IF_INTERRUPT_FLAG, IF_TIMER, true);
			}
		}
	}
}