#include "Cpu.h"
#include "Defines.h"
#include "Logger.h"
#include "Utils.h"

#include <fstream>
#include <filesystem>
#include <sstream>

namespace Core
{
	Cpu::Cpu(IMmu& mmu) : m_MMU(mmu)
	{
		// Check if the log file already exists
		if (std::filesystem::exists("cpu.txt") && m_EnableLogging)
		{
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

		ProcessTimers();

		// Each instruction takes a certain amount of cycles to complete so
		// if there are still cycles remaining then we shoud just decrement 
		// the cycles and return;
		m_Cycles--;
		if (m_Cycles > 0)
		{
			m_InstructionCompleted = false;
			return;
		}

		// read opcode from memory
		uint8_t* opcode = &m_MMU.Read(m_State.PC);

		if (m_EnableLogging)
		{
			linecount++;
			logBuffer << DisassembleInstruction(opcode);

			if (linecount >= 10000)
			{
				std::ofstream outFile("cpu.txt", std::ios::out | std::ios::app);
				outFile << logBuffer.str();
				logBuffer.str("");  // Clear the buffer
				linecount = 0;      // Reset line count
				outFile.close();
			}
		}

		if (!m_IsHalted)
		{
			// increment program counter
			m_State.PC++;

			switch (*opcode)
			{
				/********************************************************************************************
					Misc / Control Instructions
				*********************************************************************************************/

				// "NOP" B:1 C:4 FLAGS: - - - -
				case 0x00: m_Cycles = 4; break;

					// "STOP n8" B:2 C:4 FLAGS: - - - -
				case 0x10:
				{
					m_State.PC++;
					m_Cycles = 4;
					m_MMU.ResetDIVTimer();
					break;
				}

					// "HALT" B:1 C:4 FLAGS: - - - -
				case 0x76: m_IsHalted = true; m_Cycles = 4;	break;

					// "PREFIX" B:1 C:4 FLAGS: - - - -
				case 0xCB: Process16bitInstruction((opcode[0] << 8) | (opcode[1]), m_State); m_State.PC++; break;

					// "DI" B:1 C:4 FLAGS: - - - -
				case 0xF3: m_State.IME = false; m_Cycles = 4; break;

					// "EI" B:1 C:4 FLAGS: - - - -
				case 0xFB: m_State.IME = true; m_Cycles = 4; break;


					/********************************************************************************************
						Jumps/Calls
					*********************************************************************************************/

					// "JR e8" B:2 C:12 FLAGS: - - - -
				case 0x18:
				{
					m_State.PC += (int8_t)opcode[1] + 1;

					m_Cycles = 12;
					break;
				}

				// "JR NZ e8" B:2 C:12/8 FLAGS: - - - -
				case 0x20:
				{
					// note: "e8" in the description refers to a signed char
					int8_t offset = opcode[1];
					if (GetCPUFlag(FLAG_ZERO) == 0)
					{
						m_State.PC += offset + 1;
						m_Cycles = 12;
					}
					else
					{
						m_State.PC++;
						m_Cycles = 8;
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
						m_State.PC += offset + 1;
						m_Cycles = 12;
					}
					else
					{
						m_State.PC++;
						m_Cycles = 8;
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
						m_State.PC += offset + 1;
						m_Cycles = 12;
					}
					else
					{
						m_State.PC++;
						m_Cycles = 8;
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
						m_State.PC += offset + 1;
						m_Cycles = 12;
					}
					else
					{
						m_State.PC++;
						m_Cycles = 8;
					}

					break;
				}

				// "RET NZ" B:1 C:20/8 FLAGS: - - - -
				case 0xC0:
				{
					if (!GetCPUFlag(FLAG_ZERO))
					{
						m_State.PC = PopSP();
						m_Cycles = 20;
						break;
					}

					m_Cycles = 8;
					break;
				}

				// "JP NZ a16" B:3 C:16/12 FLAGS: - - - -
				case 0xC2:
				{
					if (!GetCPUFlag(FLAG_ZERO))
					{
						uint16_t offset = (opcode[2] << 8) | (opcode[1]);
						m_State.PC = offset;

						m_Cycles = 16;
						break;
					}

					m_State.PC += 2;

					m_Cycles = 12;
					break;
				}

						 // "JP a16" B:3 C:16 FLAGS: - - - -
				case 0xC3:
				{
					uint16_t offset = (opcode[2] << 8) | (opcode[1]);
					m_State.PC = offset;

					m_Cycles = 16;
					break;
				}

				// "CALL NZ a16" B:3 C:24/12 FLAGS: - - - -
				case 0xC4:
				{
					if (!GetCPUFlag(FLAG_ZERO))
					{
						PushSP(m_State.PC += 2);
						m_State.PC = (opcode[2] << 8) | (opcode[1]);

						m_Cycles = 24;
						break;
					}

					m_State.PC += 2;

					m_Cycles = 12;
					break;
				}

				// "RST $00" B:1 C:16 FLAGS: - - - -
				case 0xC7:
				{
					PushSP(m_State.PC);
					m_State.PC = 0x00;

					m_Cycles = 16;
					break;
				}

				// "RET Z" B:1 C:20/8 FLAGS: - - - -
				case 0xC8:
				{
					if (GetCPUFlag(FLAG_ZERO))
					{
						m_State.PC = PopSP();
						m_Cycles = 20;
						break;
					}

					m_Cycles = 8;
					break;
				}

				// "RET" B:1 C:16 FLAGS: - - - -
				case 0xC9:
				{
					m_State.PC = PopSP();

					m_Cycles = 16;
					break;
				}

				// "JP Z a16" B:3 C:16/12 FLAGS: - - - -
				case 0xCA:
				{
					if (GetCPUFlag(FLAG_ZERO))
					{
						uint16_t offset = (opcode[2] << 8) | (opcode[1]);
						m_State.PC = offset;

						m_Cycles = 16;
						break;
					}

					m_State.PC += 2;

					m_Cycles = 12;
					break;
				}

				// "CALL Z a16" B:3 C:24/12 FLAGS: - - - -
				case 0xCC:
				{
					if (GetCPUFlag(FLAG_ZERO))
					{
						PushSP(m_State.PC += 2);
						m_State.PC = (opcode[2] << 8) | (opcode[1]);
						m_Cycles = 24;
						break;
					}

					m_State.PC += 2;

					m_Cycles = 12;
					break;
				}

				// "CALL a16" B:3 C:24 FLAGS: - - - -
				case 0xCD:
				{
					PushSP(m_State.PC += 2);
					m_State.PC = (opcode[2] << 8) | (opcode[1]);

					m_Cycles = 24;
					break;
				}

				// "RST $08" B:1 C:16 FLAGS: - - - -
				case 0xCF:
				{
					PushSP(m_State.PC);
					m_State.PC = 0x08;

					m_Cycles = 16;
					break;
				}

				// "RET NC" B:1 C:20/8 FLAGS: - - - -
				case 0xD0:
				{
					if (!GetCPUFlag(FLAG_CARRY))
					{
						m_State.PC = PopSP();
						m_Cycles = 20;
						break;
					}

					m_Cycles = 8;
					break;
				}

				// "JP NC a16" B:3 C:16/12 FLAGS: - - - -
				case 0xD2:
				{
					if (!GetCPUFlag(FLAG_CARRY))
					{
						uint16_t offset = (opcode[2] << 8) | (opcode[1]);
						m_State.PC = offset;

						m_Cycles = 16;
						break;
					}

					m_State.PC += 2;

					m_Cycles = 12;
					break;
				}

				// "CALL NC a16" B:3 C:24/12 FLAGS: - - - -
				case 0xD4:
				{
					if (!GetCPUFlag(FLAG_CARRY))
					{
						PushSP(m_State.PC += 2);
						m_State.PC = (opcode[2] << 8) | (opcode[1]);

						m_Cycles = 24;
						break;
					}

					m_State.PC += 2;

					m_Cycles = 12;
					break;
				}

				// "RST $10" B:1 C:16 FLAGS: - - - -
				case 0xD7:
				{
					PushSP(m_State.PC);
					m_State.PC = 0x10;

					m_Cycles = 16;
					break;
				}

				// "RET C" B:1 C:20/8 FLAGS: - - - -
				case 0xD8:
				{
					if (GetCPUFlag(FLAG_CARRY))
					{
						m_State.PC = PopSP();
						m_Cycles = 20;
						break;
					}

					m_Cycles = 8;
					break;
				}

				// "RETI" B:1 C:16 FLAGS: - - - -
				case 0xD9:
				{
					m_State.PC = PopSP();
					m_State.IME = true;

					m_Cycles = 16;
					break;
				}

				// "JP C a16" B:3 C:16/12 FLAGS: - - - -
				case 0xDA:
				{
					if (GetCPUFlag(FLAG_CARRY))
					{
						uint16_t offset = (opcode[2] << 8) | (opcode[1]);
						m_State.PC = offset;

						m_Cycles = 16;
						break;
					}

					m_State.PC += 2;

					m_Cycles = 12;
					break;
				}

						 // "CALL C a16" B:3 C:24/12 FLAGS: - - - -
				case 0xDC:
				{
					if (GetCPUFlag(FLAG_CARRY))
					{
						PushSP(m_State.PC += 2);
						m_State.PC = (opcode[2] << 8) | (opcode[1]);

						m_Cycles = 24;
						break;
					}

					m_State.PC += 2;

					m_Cycles = 12;
					break;
				}

						 // "RST $18" B:1 C:16 FLAGS: - - - -
				case 0xDF:
				{
					PushSP(m_State.PC);
					m_State.PC = 0x18;

					m_Cycles = 16;
					break;
				}

				// "RST $20" B:1 C:16 FLAGS: - - - -
				case 0xE7:
				{
					PushSP(m_State.PC);
					m_State.PC = 0x20;

					m_Cycles = 16;
					break;
				}

				// "JP HL" B:1 C:4 FLAGS: - - - -
				case 0xE9:
				{
					m_State.PC = m_State.HL;

					m_Cycles = 4;
					break;
				}

				// "RST $28" B:1 C:16 FLAGS: - - - -
				case 0xEF:
				{
					PushSP(m_State.PC);
					m_State.PC = 0x28;

					m_Cycles = 16;
					break;
				}


				// "RST $30" B:1 C:16 FLAGS: - - - -
				case 0xF7:
				{
					PushSP(m_State.PC);
					m_State.PC = 0x30;

					m_Cycles = 16;
					break;
				}


				// "RST $38" B:1 C:16 FLAGS: - - - -
				case 0xFF:
				{
					PushSP(m_State.PC);
					m_State.PC = 0x38;

					m_Cycles = 16;
					break;
				}


				/********************************************************************************************
					8-bit Load Instructions
				*********************************************************************************************/

				// "LD [addr] reg" B:1 C:8 FLAGS: - - - -
				case 0x02: Instruction_ld_addr_reg(m_State.BC, m_State.A); m_Cycles = 8; break;
				case 0x12: Instruction_ld_addr_reg(m_State.DE, m_State.A); m_Cycles = 8; break;

					// "LD reg [addr]" B:1 C:8 FLAGS: - - - -
				case 0x0A: Instruction_ld_reg_addr(m_State.A, m_State.BC); m_Cycles = 8; break;
				case 0x1A: Instruction_ld_reg_addr(m_State.A, m_State.DE); m_Cycles = 8; break;
				case 0x46: Instruction_ld_reg_addr(m_State.B, m_State.HL); m_Cycles = 8; break;
				case 0x4E: Instruction_ld_reg_addr(m_State.C, m_State.HL); m_Cycles = 8; break;
				case 0x56: Instruction_ld_reg_addr(m_State.D, m_State.HL); m_Cycles = 8; break;
				case 0x5E: Instruction_ld_reg_addr(m_State.E, m_State.HL); m_Cycles = 8; break;
				case 0x66: Instruction_ld_reg_addr(m_State.H, m_State.HL); m_Cycles = 8; break;
				case 0x6E: Instruction_ld_reg_addr(m_State.L, m_State.HL); m_Cycles = 8; break;
				case 0x7E: Instruction_ld_reg_addr(m_State.A, m_State.HL); m_Cycles = 8; break;

					// "LD reg n8" B:2 C:8 FLAGS: - - - -
				case 0x06: Instruction_ld_reg_value(m_State.B, opcode[1]); m_State.PC++; m_Cycles = 8; break;
				case 0x0E: Instruction_ld_reg_value(m_State.C, opcode[1]); m_State.PC++; m_Cycles = 8; break;
				case 0x16: Instruction_ld_reg_value(m_State.D, opcode[1]); m_State.PC++; m_Cycles = 8; break;
				case 0x1E: Instruction_ld_reg_value(m_State.E, opcode[1]); m_State.PC++; m_Cycles = 8; break;
				case 0x26: Instruction_ld_reg_value(m_State.H, opcode[1]); m_State.PC++; m_Cycles = 8; break;
				case 0x2E: Instruction_ld_reg_value(m_State.L, opcode[1]); m_State.PC++; m_Cycles = 8; break;


					// "LD A [HL-]" B:1 C:8 FLAGS: - - - -
				case 0x3A:
				{
					m_State.A = m_MMU.Read(m_State.HL);
					m_State.HL--;

					m_Cycles = 8;
					break;
				}

				// "LD [HL+] A" B:1 C:8 FLAGS: - - - -
				case 0x22:
				{
					m_MMU.Write(m_State.HL, m_State.A);
					m_State.HL++;

					m_Cycles = 8;
					break;
				}

				// "LD A [HL+]" B:1 C:8 FLAGS: - - - -
				case 0x2A:
				{
					m_State.A = m_MMU.Read(m_State.HL);
					m_State.HL++;

					m_Cycles = 8;
					break;
				}

				// "LD [HL-] A" B:1 C:8 FLAGS: - - - -
				case 0x32:
				{
					m_MMU.Write(m_State.HL, m_State.A);
					m_State.HL--;

					m_Cycles = 8;
					break;
				}

				// "LD [HL] n8" B:2 C:12 FLAGS: - - - -
				case 0x36:
				{
					m_MMU.Write(m_State.HL, opcode[1]);
					m_State.PC++;

					m_Cycles = 12;
					break;
				}

				// "LD A n8" B:2 C:8 FLAGS: - - - -
				case 0x3E:
				{
					m_State.A = opcode[1];
					m_State.PC++;

					m_Cycles = 8;
					break;
				}

				// "LD reg reg" B:1 C:4 FLAGS: - - - -
				case 0x40: Instruction_ld_reg_value(m_State.B, m_State.B); m_Cycles = 4; break;
				case 0x41: Instruction_ld_reg_value(m_State.B, m_State.C); m_Cycles = 4; break;
				case 0x42: Instruction_ld_reg_value(m_State.B, m_State.D); m_Cycles = 4; break;
				case 0x43: Instruction_ld_reg_value(m_State.B, m_State.E); m_Cycles = 4; break;
				case 0x44: Instruction_ld_reg_value(m_State.B, m_State.H); m_Cycles = 4; break;
				case 0x45: Instruction_ld_reg_value(m_State.B, m_State.L); m_Cycles = 4; break;
				case 0x47: Instruction_ld_reg_value(m_State.B, m_State.A); m_Cycles = 4; break;

				case 0x48: Instruction_ld_reg_value(m_State.C, m_State.B); m_Cycles = 4; break;
				case 0x49: Instruction_ld_reg_value(m_State.C, m_State.C); m_Cycles = 4; break;
				case 0x4A: Instruction_ld_reg_value(m_State.C, m_State.D); m_Cycles = 4; break;
				case 0x4B: Instruction_ld_reg_value(m_State.C, m_State.E); m_Cycles = 4; break;
				case 0x4C: Instruction_ld_reg_value(m_State.C, m_State.H); m_Cycles = 4; break;
				case 0x4D: Instruction_ld_reg_value(m_State.C, m_State.L); m_Cycles = 4; break;
				case 0x4F: Instruction_ld_reg_value(m_State.C, m_State.A); m_Cycles = 4; break;

				case 0x50: Instruction_ld_reg_value(m_State.D, m_State.B); m_Cycles = 4; break;
				case 0x51: Instruction_ld_reg_value(m_State.D, m_State.C); m_Cycles = 4; break;
				case 0x52: Instruction_ld_reg_value(m_State.D, m_State.D); m_Cycles = 4; break;
				case 0x53: Instruction_ld_reg_value(m_State.D, m_State.E); m_Cycles = 4; break;
				case 0x54: Instruction_ld_reg_value(m_State.D, m_State.H); m_Cycles = 4; break;
				case 0x55: Instruction_ld_reg_value(m_State.D, m_State.L); m_Cycles = 4; break;
				case 0x57: Instruction_ld_reg_value(m_State.D, m_State.A); m_Cycles = 4; break;

				case 0x58: Instruction_ld_reg_value(m_State.E, m_State.B); m_Cycles = 4; break;
				case 0x59: Instruction_ld_reg_value(m_State.E, m_State.C); m_Cycles = 4; break;
				case 0x5A: Instruction_ld_reg_value(m_State.E, m_State.D); m_Cycles = 4; break;
				case 0x5B: Instruction_ld_reg_value(m_State.E, m_State.E); m_Cycles = 4; break;
				case 0x5C: Instruction_ld_reg_value(m_State.E, m_State.H); m_Cycles = 4; break;
				case 0x5D: Instruction_ld_reg_value(m_State.E, m_State.L); m_Cycles = 4; break;
				case 0x5F: Instruction_ld_reg_value(m_State.E, m_State.A); m_Cycles = 4; break;

				case 0x60: Instruction_ld_reg_value(m_State.H, m_State.B); m_Cycles = 4; break;
				case 0x61: Instruction_ld_reg_value(m_State.H, m_State.C); m_Cycles = 4; break;
				case 0x62: Instruction_ld_reg_value(m_State.H, m_State.D); m_Cycles = 4; break;
				case 0x63: Instruction_ld_reg_value(m_State.H, m_State.E); m_Cycles = 4; break;
				case 0x64: Instruction_ld_reg_value(m_State.H, m_State.H); m_Cycles = 4; break;
				case 0x65: Instruction_ld_reg_value(m_State.H, m_State.L); m_Cycles = 4; break;
				case 0x67: Instruction_ld_reg_value(m_State.H, m_State.A); m_Cycles = 4; break;

				case 0x68: Instruction_ld_reg_value(m_State.L, m_State.B); m_Cycles = 4; break;
				case 0x69: Instruction_ld_reg_value(m_State.L, m_State.C); m_Cycles = 4; break;
				case 0x6A: Instruction_ld_reg_value(m_State.L, m_State.D); m_Cycles = 4; break;
				case 0x6B: Instruction_ld_reg_value(m_State.L, m_State.E); m_Cycles = 4; break;
				case 0x6C: Instruction_ld_reg_value(m_State.L, m_State.H); m_Cycles = 4; break;
				case 0x6D: Instruction_ld_reg_value(m_State.L, m_State.L); m_Cycles = 4; break;
				case 0x6F: Instruction_ld_reg_value(m_State.L, m_State.A); m_Cycles = 4; break;

				case 0x78: Instruction_ld_reg_value(m_State.A, m_State.B); m_Cycles = 4; break;
				case 0x79: Instruction_ld_reg_value(m_State.A, m_State.C); m_Cycles = 4; break;
				case 0x7A: Instruction_ld_reg_value(m_State.A, m_State.D); m_Cycles = 4; break;
				case 0x7B: Instruction_ld_reg_value(m_State.A, m_State.E); m_Cycles = 4; break;
				case 0x7C: Instruction_ld_reg_value(m_State.A, m_State.H); m_Cycles = 4; break;
				case 0x7D: Instruction_ld_reg_value(m_State.A, m_State.L); m_Cycles = 4; break;
				case 0x7F: Instruction_ld_reg_value(m_State.A, m_State.A); m_Cycles = 4; break;

					// "LD [HL] reg" B:1 C:8 FLAGS: - - - -
				case 0x70: Instruction_ld_addr_reg(m_State.HL, m_State.B); m_Cycles = 8; break;
				case 0x71: Instruction_ld_addr_reg(m_State.HL, m_State.C); m_Cycles = 8; break;
				case 0x72: Instruction_ld_addr_reg(m_State.HL, m_State.D); m_Cycles = 8; break;
				case 0x73: Instruction_ld_addr_reg(m_State.HL, m_State.E); m_Cycles = 8; break;
				case 0x74: Instruction_ld_addr_reg(m_State.HL, m_State.H); m_Cycles = 8; break;
				case 0x75: Instruction_ld_addr_reg(m_State.HL, m_State.L); m_Cycles = 8; break;
				case 0x77: Instruction_ld_addr_reg(m_State.HL, m_State.A); m_Cycles = 8; break;



					// "LDH [a8] A" B:2 C:12 FLAGS: - - - -
				case 0xE0:
				{
					uint8_t offset = opcode[1];
					m_MMU.Write(0xFF00 + offset, m_State.A);
					m_State.PC++;

					m_Cycles = 12;
					break;
				}

				// "LD [C] A" B:1 C:8 FLAGS: - - - -
				case 0xE2:
				{
					m_MMU.Write(0xFF00 + m_State.C, m_State.A);

					m_Cycles = 8;
					break;
				}

				// "LD [a16] A" B:3 C:16 FLAGS: - - - -
				case 0xEA:
				{
					m_MMU.Write((opcode[2] << 8) | (opcode[1]), m_State.A);
					m_State.PC += 2;

					m_Cycles = 16;
					break;
				}

				// "LDH A [a8]" B:2 C:12 FLAGS: - - - -
				case 0xF0:
				{
					// just a temp hack to test the cpu output
					//uint8_t test = opcode[1];
					//if (opcode[1] == 0x44)
					//	m_State.A = 0x90;
					//else
						m_State.A = m_MMU.Read(0xFF00 + opcode[1]);

					m_State.PC++;

					m_Cycles = 12;
					break;
				}

				// "LD A [C]" B:1 C:8 FLAGS: - - - -
				case 0xF2:
				{
					m_State.A = m_MMU.Read(0xFF00 + m_State.C);

					m_Cycles = 8;
					break;
				}

				// "LD A [a16]" B:3 C:16 FLAGS: - - - -
				case 0xFA:
				{
					m_State.A = m_MMU.Read((opcode[2] << 8) | (opcode[1]));
					m_State.PC += 2;

					m_Cycles = 16;
					break;
				}


				/********************************************************************************************
					16-bit Load Instructions
				*********************************************************************************************/

				// "LD reg value" B:3 C:12 FLAGS: - - - -
				case 0x01: Instruction_ld16_reg_value(m_State.BC, (opcode[2] << 8) | (opcode[1])); m_State.PC += 2; m_Cycles = 12; break;
				case 0x11: Instruction_ld16_reg_value(m_State.DE, (opcode[2] << 8) | (opcode[1])); m_State.PC += 2; m_Cycles = 12; break;
				case 0x21: Instruction_ld16_reg_value(m_State.HL, (opcode[2] << 8) | (opcode[1])); m_State.PC += 2; m_Cycles = 12; break;
				case 0x31: Instruction_ld16_reg_value(m_State.SP, (opcode[2] << 8) | (opcode[1])); m_State.PC += 2; m_Cycles = 12; break;

					// "LD [a16] SP" B:3 C:20 FLAGS: - - - -
				case 0x08:
				{
					uint16_t addr = (opcode[2] << 8) | (opcode[1]);
					m_MMU.Write(addr, m_State.SP & 0x00FF);
					m_MMU.Write(addr + 1, (m_State.SP & 0xFF00) >> 8);
					m_State.PC += 2;

					m_Cycles = 20;
					break;
				}

				// "POP BC" B:1 C:12 FLAGS: - - - -
				case 0xC1:
				{
					m_State.BC = PopSP();

					m_Cycles = 12;
					break;
				}

				// "PUSH BC" B:1 C:16 FLAGS: - - - -
				case 0xC5:
				{
					PushSP(m_State.BC);

					m_Cycles = 16;
					break;
				}

				// "POP DE" B:1 C:12 FLAGS: - - - -
				case 0xD1:
				{
					m_State.DE = PopSP();

					m_Cycles = 12;
					break;
				}

				// "PUSH DE" B:1 C:16 FLAGS: - - - -
				case 0xD5:
				{
					PushSP(m_State.DE);

					m_Cycles = 16;
					break;
				}

				// "POP HL" B:1 C:12 FLAGS: - - - -
				case 0xE1:
				{
					m_State.HL = PopSP();

					m_Cycles = 12;
					break;
				}

				// "PUSH HL" B:1 C:16 FLAGS: - - - -
				case 0xE5:
				{
					PushSP(m_State.HL);

					m_Cycles = 16;
					break;
				}

				// "POP AF" B:1 C:12 FLAGS: Z N H C
				case 0xF1:
				{
					m_State.AF = (PopSP() & 0xFFF0);

					m_Cycles = 12;
					break;
				}

				// "PUSH AF" B:1 C:16 FLAGS: - - - -
				case 0xF5:
				{
					PushSP(m_State.AF);

					m_Cycles = 16;
					break;
				}

				// "LD HL SP e8" B:2 C:12 FLAGS: 0 0 H C
				case 0xF8:
				{
					int8_t signedValue = static_cast<int8_t>(opcode[1]);
					int32_t fullResult = m_State.SP + signedValue;
					uint16_t result = static_cast<uint16_t>(fullResult);

					SetCPUFlag(FLAG_ZERO, false);
					SetCPUFlag(FLAG_SUBTRACT, false);
					SetCPUFlag(FLAG_HALF_CARRY, ((m_State.SP ^ signedValue ^ (fullResult & 0xFFFF)) & 0x10) == 0x10);
					SetCPUFlag(FLAG_CARRY, ((m_State.SP ^ signedValue ^ (fullResult & 0xFFFF)) & 0x100) == 0x100);

					m_State.HL = result;
					m_State.PC++;

					m_Cycles = 12;
					break;
				}

				// "LD SP HL" B:1 C:8 FLAGS: - - - -
				case 0xF9:
				{
					m_State.SP = m_State.HL;

					m_Cycles = 8;
					break;
				}


				/********************************************************************************************
					8-bit Arithmetic/Logical Instructions
				*********************************************************************************************/

				// "INC reg" B:1 C:4 FLAGS: Z 0 H -
				case 0x04: Instruction_inc_reg(m_State.B); m_Cycles = 4; break;
				case 0x0C: Instruction_inc_reg(m_State.C); m_Cycles = 4; break;
				case 0x14: Instruction_inc_reg(m_State.D); m_Cycles = 4; break;
				case 0x1C: Instruction_inc_reg(m_State.E); m_Cycles = 4; break;
				case 0x24: Instruction_inc_reg(m_State.H); m_Cycles = 4; break;
				case 0x2C: Instruction_inc_reg(m_State.L); m_Cycles = 4; break;
				case 0x34: Instruction_inc_hl(); m_Cycles = 12; break;
				case 0x3C: Instruction_inc_reg(m_State.A); m_Cycles = 4; break;

					// "DEC reg" B:1 C:4 FLAGS: Z 1 H -
				case 0x05: Instruction_dec_reg(m_State.B); m_Cycles = 4; break;
				case 0x0D: Instruction_dec_reg(m_State.C); m_Cycles = 4; break;
				case 0x15: Instruction_dec_reg(m_State.D); m_Cycles = 4; break;
				case 0x1D: Instruction_dec_reg(m_State.E); m_Cycles = 4; break;
				case 0x25: Instruction_dec_reg(m_State.H); m_Cycles = 4; break;
				case 0x2D: Instruction_dec_reg(m_State.L); m_Cycles = 4; break;
				case 0x35: Instruction_dec_hl(); m_Cycles = 12; break;
				case 0x3D: Instruction_dec_reg(m_State.A); m_Cycles = 4; break;

					// "ADD A reg" B:1 C:4 FLAGS: Z 0 H C
				case 0x80: Instruction_add_reg(m_State.B); m_Cycles = 4; break;
				case 0x81: Instruction_add_reg(m_State.C); m_Cycles = 4; break;
				case 0x82: Instruction_add_reg(m_State.D); m_Cycles = 4; break;
				case 0x83: Instruction_add_reg(m_State.E); m_Cycles = 4; break;
				case 0x84: Instruction_add_reg(m_State.H); m_Cycles = 4; break;
				case 0x85: Instruction_add_reg(m_State.L); m_Cycles = 4; break;
				case 0x86: Instruction_add_hl(); m_Cycles = 8; break;
				case 0x87: Instruction_add_reg(m_State.A); m_Cycles = 4; break;
				case 0xC6: Instruction_add_reg(opcode[1]); m_State.PC++; m_Cycles = 8; break;

					// "ADC A reg" B:1 C:4 FLAGS: Z 0 H C
				case 0x88: Instruction_adc_reg(m_State.B); m_Cycles = 4; break;
				case 0x89: Instruction_adc_reg(m_State.C); m_Cycles = 4; break;
				case 0x8A: Instruction_adc_reg(m_State.D); m_Cycles = 4; break;
				case 0x8B: Instruction_adc_reg(m_State.E); m_Cycles = 4; break;
				case 0x8C: Instruction_adc_reg(m_State.H); m_Cycles = 4; break;
				case 0x8D: Instruction_adc_reg(m_State.L); m_Cycles = 4; break;
				case 0x8E: Instruction_adc_hl(); m_Cycles = 8; break;
				case 0x8F: Instruction_adc_reg(m_State.A); m_Cycles = 4; break;
				case 0xCE: Instruction_adc_reg(opcode[1]); m_State.PC++; m_Cycles = 8; break;

					// "SUB A reg" B:1 C:4 FLAGS: Z 1 H C
				case 0x90: Instruction_sub_reg(m_State.B); m_Cycles = 4; break;
				case 0x91: Instruction_sub_reg(m_State.C); m_Cycles = 4; break;
				case 0x92: Instruction_sub_reg(m_State.D); m_Cycles = 4; break;
				case 0x93: Instruction_sub_reg(m_State.E); m_Cycles = 4; break;
				case 0x94: Instruction_sub_reg(m_State.H); m_Cycles = 4; break;
				case 0x95: Instruction_sub_reg(m_State.L); m_Cycles = 4; break;
				case 0x96: Instruction_sub_hl(); m_Cycles = 8; break;
				case 0x97:
				{
					// SUB A A is a special case where the flags need to be 1 1 0 0
					Instruction_sub_reg(m_State.A);
					SetCPUFlag(FLAG_ZERO, true);
					SetCPUFlag(FLAG_SUBTRACT, true);
					SetCPUFlag(FLAG_HALF_CARRY, false);
					SetCPUFlag(FLAG_CARRY, false);
					m_Cycles = 4;
					break;
				}
				case 0xD6: Instruction_sub_reg(opcode[1]); m_State.PC++; m_Cycles = 8; break;

					// "SBC A reg" B:1 C:4 FLAGS: Z 1 H C
				case 0x98: Instruction_sbc_reg(m_State.B); m_Cycles = 4; break;
				case 0x99: Instruction_sbc_reg(m_State.C); m_Cycles = 4; break;
				case 0x9A: Instruction_sbc_reg(m_State.D); m_Cycles = 4; break;
				case 0x9B: Instruction_sbc_reg(m_State.E); m_Cycles = 4; break;
				case 0x9C: Instruction_sbc_reg(m_State.H); m_Cycles = 4; break;
				case 0x9D: Instruction_sbc_reg(m_State.L); m_Cycles = 4; break;
				case 0x9E: Instruction_sbc_hl(); m_Cycles = 8; break;
				case 0x9F: Instruction_sbc_reg(m_State.A); m_Cycles = 4; break;
				case 0xDE: Instruction_sbc_reg(opcode[1]); m_State.PC++; m_Cycles = 8; break;

					// "AND A reg" B:1 C:4 FLAGS: Z 0 1 0
				case 0xA0: Instruction_and_reg(m_State.B); m_Cycles = 4; break;
				case 0xA1: Instruction_and_reg(m_State.C); m_Cycles = 4; break;
				case 0xA2: Instruction_and_reg(m_State.D); m_Cycles = 4; break;
				case 0xA3: Instruction_and_reg(m_State.E); m_Cycles = 4; break;
				case 0xA4: Instruction_and_reg(m_State.H); m_Cycles = 4; break;
				case 0xA5: Instruction_and_reg(m_State.L); m_Cycles = 4; break;
				case 0xA6: Instruction_and_hl(); m_Cycles = 8; break;
				case 0xA7: Instruction_and_reg(m_State.A); m_Cycles = 4; break;
				case 0xE6: Instruction_and_reg(opcode[1]); m_State.PC++; m_Cycles = 8; break;

					// "XOR A reg" B:1 C:4 FLAGS: Z 0 0 0
				case 0xA8: Instruction_xor_reg(m_State.B); m_Cycles = 4; break;
				case 0xA9: Instruction_xor_reg(m_State.C); m_Cycles = 4; break;
				case 0xAA: Instruction_xor_reg(m_State.D); m_Cycles = 4; break;
				case 0xAB: Instruction_xor_reg(m_State.E); m_Cycles = 4; break;
				case 0xAC: Instruction_xor_reg(m_State.H); m_Cycles = 4; break;
				case 0xAD: Instruction_xor_reg(m_State.L); m_Cycles = 4; break;
				case 0xAE: Instruction_xor_hl(); m_Cycles = 8; break;
				case 0xAF: Instruction_xor_reg(m_State.A); m_Cycles = 4; break;
				case 0xEE: Instruction_xor_reg(opcode[1]); m_State.PC++; m_Cycles = 8; break;

					// "OR A reg" B:1 C:4 FLAGS: Z 0 0 0
				case 0xB0: Instruction_or_reg(m_State.B); m_Cycles = 4; break;
				case 0xB1: Instruction_or_reg(m_State.C); m_Cycles = 4; break;
				case 0xB2: Instruction_or_reg(m_State.D); m_Cycles = 4; break;
				case 0xB3: Instruction_or_reg(m_State.E); m_Cycles = 4; break;
				case 0xB4: Instruction_or_reg(m_State.H); m_Cycles = 4; break;
				case 0xB5: Instruction_or_reg(m_State.L); m_Cycles = 4; break;
				case 0xB6: Instruction_or_hl(); m_Cycles = 8; break;
				case 0xB7: Instruction_or_reg(m_State.A); m_Cycles = 4; break;
				case 0xF6: Instruction_or_reg(opcode[1]); m_State.PC++; m_Cycles = 8; break;

					// "CP A B" B:1 C:4 FLAGS: Z 1 H C
				case 0xB8: Instruction_cp_reg(m_State.B); m_Cycles = 4; break;
				case 0xB9: Instruction_cp_reg(m_State.C); m_Cycles = 4; break;
				case 0xBA: Instruction_cp_reg(m_State.D); m_Cycles = 4; break;
				case 0xBB: Instruction_cp_reg(m_State.E); m_Cycles = 4; break;
				case 0xBC: Instruction_cp_reg(m_State.H); m_Cycles = 4; break;
				case 0xBD: Instruction_cp_reg(m_State.L); m_Cycles = 4; break;
				case 0xBE: Instruction_cp_hl(); m_Cycles = 8; break;
				case 0xBF:
				{
					// CP A A is a special case where the flags need to be 1 1 0 0
					SetCPUFlag(FLAG_ZERO, true);
					SetCPUFlag(FLAG_SUBTRACT, true);
					SetCPUFlag(FLAG_HALF_CARRY, false);
					SetCPUFlag(FLAG_CARRY, false);

					m_Cycles = 4;
					break;
				}
				case 0xFE: Instruction_cp_reg(opcode[1]); m_State.PC++; m_Cycles = 8; break;


					/********************************************************************************************
						16-bit Arithmetic/Logical Instructions
					*********************************************************************************************/

					// "INC reg16" B:1 C:8 FLAGS: - - - -
				case 0x03: Instruction_inc_reg16(m_State.BC); m_Cycles = 8; break;
				case 0x13: Instruction_inc_reg16(m_State.DE); m_Cycles = 8; break;
				case 0x23: Instruction_inc_reg16(m_State.HL); m_Cycles = 8; break;
				case 0x33: Instruction_inc_reg16(m_State.SP); m_Cycles = 8; break;

					// "DEC reg16" B:1 C:8 FLAGS: - - - -
				case 0x0B: Instruction_dec_reg16(m_State.BC); m_Cycles = 8; break;
				case 0x1B: Instruction_dec_reg16(m_State.DE); m_Cycles = 8; break;
				case 0x2B: Instruction_dec_reg16(m_State.HL); m_Cycles = 8; break;
				case 0x3B: Instruction_dec_reg16(m_State.SP); m_Cycles = 8; break;

					// "ADD HL reg16" B:1 C:8 FLAGS: - 0 H C
				case 0x09: Instruction_add_reg16(m_State.BC); m_Cycles = 8; break;
				case 0x19: Instruction_add_reg16(m_State.DE); m_Cycles = 8; break;
				case 0x29: Instruction_add_reg16(m_State.HL); m_Cycles = 8; break;
				case 0x39: Instruction_add_reg16(m_State.SP); m_Cycles = 8; break;

					// "ADD SP e8" B:2 C:16 FLAGS: 0 0 H C
				case 0xE8: Instruction_add_sp_e8(opcode[1]); m_State.PC++; m_Cycles = 16; break;

					/********************************************************************************************
						8-bit Shift. Rotate and Bit Instructions
					*********************************************************************************************/

					// "RLCA" B:1 C:4 FLAGS: 0 0 0 C
				case 0x07:
				{
					uint8_t result = (m_State.A << 1) | ((m_State.A >> 7) & 0x1);

					SetCPUFlag(FLAG_ZERO, false);
					SetCPUFlag(FLAG_SUBTRACT, false);
					SetCPUFlag(FLAG_HALF_CARRY, false);
					SetCPUFlag(FLAG_CARRY, ((m_State.A >> 7) & 0x1) != 0);

					m_State.A = result;

					m_Cycles = 4;
					break;
				}

				// "RRCA" B:1 C:4 FLAGS: 0 0 0 C
				case 0x0F:
				{
					uint8_t result = ((m_State.A & 0x1) << 7) | (m_State.A >> 1);

					SetCPUFlag(FLAG_ZERO, false);
					SetCPUFlag(FLAG_SUBTRACT, false);
					SetCPUFlag(FLAG_HALF_CARRY, false);
					SetCPUFlag(FLAG_CARRY, (m_State.A & 0x1) != 0);

					m_State.A = result;

					m_Cycles = 4;
					break;
				}

				// "RLA" B:1 C:4 FLAGS: 0 0 0 C
				case 0x17:
				{
					uint8_t result = (m_State.A << 1) | (GetCPUFlag(FLAG_CARRY) ? 1 : 0);
					bool carry = (m_State.A & 0x80) != 0;
					m_State.A = result;

					SetCPUFlag(FLAG_ZERO, false);
					SetCPUFlag(FLAG_SUBTRACT, false);
					SetCPUFlag(FLAG_HALF_CARRY, false);
					SetCPUFlag(FLAG_CARRY, carry);

					m_Cycles = 4;
					break;
				}

				// "RRA" B:1 C:4 FLAGS: 0 0 0 C
				case 0x1F:
				{
					uint8_t carry = GetCPUFlag(FLAG_CARRY) ? 0x80 : 0;
					uint8_t result = carry | (m_State.A >> 1);

					SetCPUFlag(FLAG_ZERO, false);
					SetCPUFlag(FLAG_SUBTRACT, false);
					SetCPUFlag(FLAG_HALF_CARRY, false);
					SetCPUFlag(FLAG_CARRY, (m_State.A & 0x1) != 0);

					m_State.A = result;

					m_Cycles = 4;
					break;
				}

				// "DAA" B:1 C:4 FLAGS: Z - 0 C
				case 0x27:
				{
					int32_t result = m_State.A;
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
						if (GetCPUFlag(FLAG_HALF_CARRY) || (m_State.A & 0x0F) > 9)
						{
							result += 6;
						}

						if (GetCPUFlag(FLAG_CARRY) || result > 0x9F)
						{
							result += 0x60;
						}
					}

					m_State.A = static_cast<uint8_t>(result);

					if (result > 0xFF)
						SetCPUFlag(FLAG_CARRY, true);

					SetCPUFlag(FLAG_ZERO, m_State.A == 0);
					SetCPUFlag(FLAG_HALF_CARRY, false);

					m_Cycles = 4;
					break;
				}

				// "CPL" B:1 C:4 FLAGS: - 1 1 -
				case 0x2F:
				{
					m_State.A = ~m_State.A;
					SetCPUFlag(FLAG_SUBTRACT, true);
					SetCPUFlag(FLAG_HALF_CARRY, true);

					m_Cycles = 4;
					break;
				}

				// "SCF" B:1 C:4 FLAGS: - 0 0 1
				case 0x37:
				{
					SetCPUFlag(FLAG_SUBTRACT, false);
					SetCPUFlag(FLAG_HALF_CARRY, false);
					SetCPUFlag(FLAG_CARRY, true);

					m_Cycles = 4;
					break;
				}

				// "CCF" B:1 C:4 FLAGS: - 0 0 C
				case 0x3F:
				{
					// flip the carry flag
					GetCPUFlag(FLAG_CARRY) == true ? SetCPUFlag(FLAG_CARRY, false) : SetCPUFlag(FLAG_CARRY, true);

					SetCPUFlag(FLAG_SUBTRACT, false);
					SetCPUFlag(FLAG_HALF_CARRY, false);

					m_Cycles = 4;
					break;
				}

				default:
					Logger::Instance().Error(Domain::CPU, "Error: Unimplemented instruction");
					break;
			}
		}

		ProcessInterrupts();
		m_InstructionCompleted = true;
	}

	void Cpu::ProcessInterrupts()
	{
		uint16_t destinationAddress = 0;
		uint8_t interruptFlag = 0;

		if (m_MMU.ReadRegisterBit(HW_FFFF_INTERRUPT_ENABLE, IE_VBLANK) &&
			m_MMU.ReadRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, IF_VBLANK))
		{
			destinationAddress = DEST_ADDRESS_VBLANK;
			interruptFlag = IF_VBLANK;
			m_IsHalted = false;
		}
		else if (m_MMU.ReadRegisterBit(HW_FFFF_INTERRUPT_ENABLE, IE_LCD) &&
			m_MMU.ReadRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, IF_LCD))
		{
			destinationAddress = DEST_ADDRESS_LCD_STAT;
			interruptFlag = IF_LCD;
			m_IsHalted = false;
		}
		else if (m_MMU.ReadRegisterBit(HW_FFFF_INTERRUPT_ENABLE, IE_TIMER) &&
			m_MMU.ReadRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, IF_TIMER))
		{
			destinationAddress = DEST_ADDRESS_TIMER;
			interruptFlag = IF_TIMER;
			m_IsHalted = false;
		}
		else if (m_MMU.ReadRegisterBit(HW_FFFF_INTERRUPT_ENABLE, IE_SERIAL) &&
			m_MMU.ReadRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, IF_SERIAL))
		{
			destinationAddress = DEST_ADDRESS_SERIAL;
			interruptFlag = IF_SERIAL;
			m_IsHalted = false;
		}
		else if (m_MMU.ReadRegisterBit(HW_FFFF_INTERRUPT_ENABLE, IE_JOYPAD) &&
			m_MMU.ReadRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, IF_JOYPAD))
		{
			destinationAddress = DEST_ADDRESS_JOYPAD;
			interruptFlag = IF_JOYPAD;
			m_IsHalted = false;
		}

		if (destinationAddress != 0 && m_State.IME)
		{
			m_State.IME = false;;
			m_MMU.WriteRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, interruptFlag, false);

			PushSP(m_State.PC);
			m_State.PC = destinationAddress;

			m_Cycles = 20;

			std::ostringstream stream;
			stream << "IRQ Write - Address: " << std::hex << destinationAddress << " Interrupt Type: ";
			Logger::Instance().Verbose(Domain::CPU, stream.str());
		}
	}

	void Cpu::Process16bitInstruction(uint16_t opcode, sCPUState& state)
	{
		switch (opcode)
		{
			// Most instuctions take 8 cycles
			m_Cycles = 8;

			// "RLC reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB00: Instruction_rlc_reg(m_State.B); break;
			case 0xCB01: Instruction_rlc_reg(m_State.C); break;
			case 0xCB02: Instruction_rlc_reg(m_State.D); break;
			case 0xCB03: Instruction_rlc_reg(m_State.E); break;
			case 0xCB04: Instruction_rlc_reg(m_State.H); break;
			case 0xCB05: Instruction_rlc_reg(m_State.L); break;
			case 0xCB06: Instruction_rlc_hl(); m_Cycles = 16; break;
			case 0xCB07: Instruction_rlc_reg(m_State.A); break;

				// "RRC reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB08: Instruction_rrc_reg(m_State.B); break;
			case 0xCB09: Instruction_rrc_reg(m_State.C); break;
			case 0xCB0A: Instruction_rrc_reg(m_State.D); break;
			case 0xCB0B: Instruction_rrc_reg(m_State.E); break;
			case 0xCB0C: Instruction_rrc_reg(m_State.H); break;
			case 0xCB0D: Instruction_rrc_reg(m_State.L); break;
			case 0xCB0E: Instruction_rrc_hl(); m_Cycles = 16; break;
			case 0xCB0F: Instruction_rrc_reg(m_State.A); break;

				// "RL reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB10: Instruction_rl_reg(m_State.B); break;
			case 0xCB11: Instruction_rl_reg(m_State.C); break;
			case 0xCB12: Instruction_rl_reg(m_State.D); break;
			case 0xCB13: Instruction_rl_reg(m_State.E); break;
			case 0xCB14: Instruction_rl_reg(m_State.H); break;
			case 0xCB15: Instruction_rl_reg(m_State.L); break;
			case 0xCB16: Instruction_rl_hl(); m_Cycles = 16; break;
			case 0xCB17: Instruction_rl_reg(m_State.A); break;

				// "RR reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB18: Instruction_rr_reg(m_State.B); break;
			case 0xCB19: Instruction_rr_reg(m_State.C); break;
			case 0xCB1A: Instruction_rr_reg(m_State.D); break;
			case 0xCB1B: Instruction_rr_reg(m_State.E); break;
			case 0xCB1C: Instruction_rr_reg(m_State.H); break;
			case 0xCB1D: Instruction_rr_reg(m_State.L); break;
			case 0xCB1E: Instruction_rr_hl(); m_Cycles = 16; break;
			case 0xCB1F: Instruction_rr_reg(m_State.A); break;

				// "SLA reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB20: Instruction_sla_reg(m_State.B); break;
			case 0xCB21: Instruction_sla_reg(m_State.C); break;
			case 0xCB22: Instruction_sla_reg(m_State.D); break;
			case 0xCB23: Instruction_sla_reg(m_State.E); break;
			case 0xCB24: Instruction_sla_reg(m_State.H); break;
			case 0xCB25: Instruction_sla_reg(m_State.L); break;
			case 0xCB26: Instruction_sla_hl(); m_Cycles = 16; break;
			case 0xCB27: Instruction_sla_reg(m_State.A); break;

				// "SRA reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB28: Instruction_sra_reg(m_State.B); break;
			case 0xCB29: Instruction_sra_reg(m_State.C); break;
			case 0xCB2A: Instruction_sra_reg(m_State.D); break;
			case 0xCB2B: Instruction_sra_reg(m_State.E); break;
			case 0xCB2C: Instruction_sra_reg(m_State.H); break;
			case 0xCB2D: Instruction_sra_reg(m_State.L); break;
			case 0xCB2E: Instruction_sra_hl(); m_Cycles = 16; break;
			case 0xCB2F: Instruction_sra_reg(m_State.A); break;

				// "SWAP reg" B:2 C:8 FLAGS: Z 0 0 0
			case 0xCB30: Instruction_swap_reg(m_State.B); break;
			case 0xCB31: Instruction_swap_reg(m_State.C); break;
			case 0xCB32: Instruction_swap_reg(m_State.D); break;
			case 0xCB33: Instruction_swap_reg(m_State.E); break;
			case 0xCB34: Instruction_swap_reg(m_State.H); break;
			case 0xCB35: Instruction_swap_reg(m_State.L); break;
			case 0xCB36: Instruction_swap_hl(); m_Cycles = 16; break;
			case 0xCB37: Instruction_swap_reg(m_State.A); break;

				// "SRL bit" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB38: Instruction_srl_reg(m_State.B); break;
			case 0xCB39: Instruction_srl_reg(m_State.C); break;
			case 0xCB3A: Instruction_srl_reg(m_State.D); break;
			case 0xCB3B: Instruction_srl_reg(m_State.E); break;
			case 0xCB3C: Instruction_srl_reg(m_State.H); break;
			case 0xCB3D: Instruction_srl_reg(m_State.L); break;
			case 0xCB3E: Instruction_srl_hl(); m_Cycles = 16; break;
			case 0xCB3F: Instruction_srl_reg(m_State.A); break;

				// "BIT bit, reg" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB40: Instruction_bit_bit_reg(m_State.B, 0); break;
			case 0xCB41: Instruction_bit_bit_reg(m_State.C, 0); break;
			case 0xCB42: Instruction_bit_bit_reg(m_State.D, 0); break;
			case 0xCB43: Instruction_bit_bit_reg(m_State.E, 0); break;
			case 0xCB44: Instruction_bit_bit_reg(m_State.H, 0); break;
			case 0xCB45: Instruction_bit_bit_reg(m_State.L, 0); break;
			case 0xCB46: Instruction_bit_bit_hl(0); m_Cycles = 12; break;
			case 0xCB47: Instruction_bit_bit_reg(m_State.A, 0); break;
			case 0xCB48: Instruction_bit_bit_reg(m_State.B, 1); break;
			case 0xCB49: Instruction_bit_bit_reg(m_State.C, 1); break;
			case 0xCB4A: Instruction_bit_bit_reg(m_State.D, 1); break;
			case 0xCB4B: Instruction_bit_bit_reg(m_State.E, 1); break;
			case 0xCB4C: Instruction_bit_bit_reg(m_State.H, 1); break;
			case 0xCB4D: Instruction_bit_bit_reg(m_State.L, 1); break;
			case 0xCB4E: Instruction_bit_bit_hl(1); m_Cycles = 12; break;
			case 0xCB4F: Instruction_bit_bit_reg(m_State.A, 1); break;
			case 0xCB50: Instruction_bit_bit_reg(m_State.B, 2); break;
			case 0xCB51: Instruction_bit_bit_reg(m_State.C, 2); break;
			case 0xCB52: Instruction_bit_bit_reg(m_State.D, 2); break;
			case 0xCB53: Instruction_bit_bit_reg(m_State.E, 2); break;
			case 0xCB54: Instruction_bit_bit_reg(m_State.H, 2); break;
			case 0xCB55: Instruction_bit_bit_reg(m_State.L, 2); break;
			case 0xCB56: Instruction_bit_bit_hl(2); m_Cycles = 12; break;
			case 0xCB57: Instruction_bit_bit_reg(m_State.A, 2); break;
			case 0xCB58: Instruction_bit_bit_reg(m_State.B, 3); break;
			case 0xCB59: Instruction_bit_bit_reg(m_State.C, 3); break;
			case 0xCB5A: Instruction_bit_bit_reg(m_State.D, 3); break;
			case 0xCB5B: Instruction_bit_bit_reg(m_State.E, 3); break;
			case 0xCB5C: Instruction_bit_bit_reg(m_State.H, 3); break;
			case 0xCB5D: Instruction_bit_bit_reg(m_State.L, 3); break;
			case 0xCB5E: Instruction_bit_bit_hl(3); m_Cycles = 12; break;
			case 0xCB5F: Instruction_bit_bit_reg(m_State.A, 3); break;
			case 0xCB60: Instruction_bit_bit_reg(m_State.B, 4); break;
			case 0xCB61: Instruction_bit_bit_reg(m_State.C, 4); break;
			case 0xCB62: Instruction_bit_bit_reg(m_State.D, 4); break;
			case 0xCB63: Instruction_bit_bit_reg(m_State.E, 4); break;
			case 0xCB64: Instruction_bit_bit_reg(m_State.H, 4); break;
			case 0xCB65: Instruction_bit_bit_reg(m_State.L, 4); break;
			case 0xCB66: Instruction_bit_bit_hl(4); m_Cycles = 12; break;
			case 0xCB67: Instruction_bit_bit_reg(m_State.A, 4); break;
			case 0xCB68: Instruction_bit_bit_reg(m_State.B, 5); break;
			case 0xCB69: Instruction_bit_bit_reg(m_State.C, 5); break;
			case 0xCB6A: Instruction_bit_bit_reg(m_State.D, 5); break;
			case 0xCB6B: Instruction_bit_bit_reg(m_State.E, 5); break;
			case 0xCB6C: Instruction_bit_bit_reg(m_State.H, 5); break;
			case 0xCB6D: Instruction_bit_bit_reg(m_State.L, 5); break;
			case 0xCB6E: Instruction_bit_bit_hl(5); m_Cycles = 12; break;
			case 0xCB6F: Instruction_bit_bit_reg(m_State.A, 5); break;
			case 0xCB70: Instruction_bit_bit_reg(m_State.B, 6); break;
			case 0xCB71: Instruction_bit_bit_reg(m_State.C, 6); break;
			case 0xCB72: Instruction_bit_bit_reg(m_State.D, 6); break;
			case 0xCB73: Instruction_bit_bit_reg(m_State.E, 6); break;
			case 0xCB74: Instruction_bit_bit_reg(m_State.H, 6); break;
			case 0xCB75: Instruction_bit_bit_reg(m_State.L, 6); break;
			case 0xCB76: Instruction_bit_bit_hl(6); m_Cycles = 12; break;
			case 0xCB77: Instruction_bit_bit_reg(m_State.A, 6); break;
			case 0xCB78: Instruction_bit_bit_reg(m_State.B, 7); break;
			case 0xCB79: Instruction_bit_bit_reg(m_State.C, 7); break;
			case 0xCB7A: Instruction_bit_bit_reg(m_State.D, 7); break;
			case 0xCB7B: Instruction_bit_bit_reg(m_State.E, 7); break;
			case 0xCB7C: Instruction_bit_bit_reg(m_State.H, 7); break;
			case 0xCB7D: Instruction_bit_bit_reg(m_State.L, 7); break;
			case 0xCB7E: Instruction_bit_bit_hl(7); m_Cycles = 12; break;
			case 0xCB7F: Instruction_bit_bit_reg(m_State.A, 7); break;

				// "RES bit, reg" B:2 C:8 FLAGS: - - - -
			case 0xCB80: Instruction_res_bit_reg(m_State.B, 0); break;
			case 0xCB81: Instruction_res_bit_reg(m_State.C, 0); break;
			case 0xCB82: Instruction_res_bit_reg(m_State.D, 0); break;
			case 0xCB83: Instruction_res_bit_reg(m_State.E, 0); break;
			case 0xCB84: Instruction_res_bit_reg(m_State.H, 0); break;
			case 0xCB85: Instruction_res_bit_reg(m_State.L, 0); break;
			case 0xCB86: Instruction_res_bit_hl(0); m_Cycles = 16; break;
			case 0xCB87: Instruction_res_bit_reg(m_State.A, 0); break;
			case 0xCB88: Instruction_res_bit_reg(m_State.B, 1); break;
			case 0xCB89: Instruction_res_bit_reg(m_State.C, 1); break;
			case 0xCB8A: Instruction_res_bit_reg(m_State.D, 1); break;
			case 0xCB8B: Instruction_res_bit_reg(m_State.E, 1); break;
			case 0xCB8C: Instruction_res_bit_reg(m_State.H, 1); break;
			case 0xCB8D: Instruction_res_bit_reg(m_State.L, 1); break;
			case 0xCB8E: Instruction_res_bit_hl(1); m_Cycles = 16; break;
			case 0xCB8F: Instruction_res_bit_reg(m_State.A, 1); break;
			case 0xCB90: Instruction_res_bit_reg(m_State.B, 2); break;
			case 0xCB91: Instruction_res_bit_reg(m_State.C, 2); break;
			case 0xCB92: Instruction_res_bit_reg(m_State.D, 2); break;
			case 0xCB93: Instruction_res_bit_reg(m_State.E, 2); break;
			case 0xCB94: Instruction_res_bit_reg(m_State.H, 2); break;
			case 0xCB95: Instruction_res_bit_reg(m_State.L, 2); break;
			case 0xCB96: Instruction_res_bit_hl(2); m_Cycles = 16; break;
			case 0xCB97: Instruction_res_bit_reg(m_State.A, 2); break;
			case 0xCB98: Instruction_res_bit_reg(m_State.B, 3); break;
			case 0xCB99: Instruction_res_bit_reg(m_State.C, 3); break;
			case 0xCB9A: Instruction_res_bit_reg(m_State.D, 3); break;
			case 0xCB9B: Instruction_res_bit_reg(m_State.E, 3); break;
			case 0xCB9C: Instruction_res_bit_reg(m_State.H, 3); break;
			case 0xCB9D: Instruction_res_bit_reg(m_State.L, 3); break;
			case 0xCB9E: Instruction_res_bit_hl(3); m_Cycles = 16; break;
			case 0xCB9F: Instruction_res_bit_reg(m_State.A, 3); break;
			case 0xCBA0: Instruction_res_bit_reg(m_State.B, 4); break;
			case 0xCBA1: Instruction_res_bit_reg(m_State.C, 4); break;
			case 0xCBA2: Instruction_res_bit_reg(m_State.D, 4); break;
			case 0xCBA3: Instruction_res_bit_reg(m_State.E, 4); break;
			case 0xCBA4: Instruction_res_bit_reg(m_State.H, 4); break;
			case 0xCBA5: Instruction_res_bit_reg(m_State.L, 4); break;
			case 0xCBA6: Instruction_res_bit_hl(4); m_Cycles = 16; break;
			case 0xCBA7: Instruction_res_bit_reg(m_State.A, 4); break;
			case 0xCBA8: Instruction_res_bit_reg(m_State.B, 5); break;
			case 0xCBA9: Instruction_res_bit_reg(m_State.C, 5); break;
			case 0xCBAA: Instruction_res_bit_reg(m_State.D, 5); break;
			case 0xCBAB: Instruction_res_bit_reg(m_State.E, 5); break;
			case 0xCBAC: Instruction_res_bit_reg(m_State.H, 5); break;
			case 0xCBAD: Instruction_res_bit_reg(m_State.L, 5); break;
			case 0xCBAE: Instruction_res_bit_hl(5); m_Cycles = 16; break;
			case 0xCBAF: Instruction_res_bit_reg(m_State.A, 5); break;
			case 0xCBB0: Instruction_res_bit_reg(m_State.B, 6); break;
			case 0xCBB1: Instruction_res_bit_reg(m_State.C, 6); break;
			case 0xCBB2: Instruction_res_bit_reg(m_State.D, 6); break;
			case 0xCBB3: Instruction_res_bit_reg(m_State.E, 6); break;
			case 0xCBB4: Instruction_res_bit_reg(m_State.H, 6); break;
			case 0xCBB5: Instruction_res_bit_reg(m_State.L, 6); break;
			case 0xCBB6: Instruction_res_bit_hl(6); m_Cycles = 16; break;
			case 0xCBB7: Instruction_res_bit_reg(m_State.A, 6); break;
			case 0xCBB8: Instruction_res_bit_reg(m_State.B, 7); break;
			case 0xCBB9: Instruction_res_bit_reg(m_State.C, 7); break;
			case 0xCBBA: Instruction_res_bit_reg(m_State.D, 7); break;
			case 0xCBBB: Instruction_res_bit_reg(m_State.E, 7); break;
			case 0xCBBC: Instruction_res_bit_reg(m_State.H, 7); break;
			case 0xCBBD: Instruction_res_bit_reg(m_State.L, 7); break;
			case 0xCBBE: Instruction_res_bit_hl(7); m_Cycles = 16; break;
			case 0xCBBF: Instruction_res_bit_reg(m_State.A, 7); break;

				// "SET bit, register" B:2 C:8 - - - -
			case 0xCBC0: Instruction_set_bit_reg(m_State.B, 0); break;
			case 0xCBC1: Instruction_set_bit_reg(m_State.C, 0); break;
			case 0xCBC2: Instruction_set_bit_reg(m_State.D, 0); break;
			case 0xCBC3: Instruction_set_bit_reg(m_State.E, 0); break;
			case 0xCBC4: Instruction_set_bit_reg(m_State.H, 0); break;
			case 0xCBC5: Instruction_set_bit_reg(m_State.L, 0); break;
			case 0xCBC6: Instruction_set_bit_hl(0); m_Cycles = 16; break;
			case 0xCBC7: Instruction_set_bit_reg(m_State.A, 0); break;
			case 0xCBC8: Instruction_set_bit_reg(m_State.B, 1); break;
			case 0xCBC9: Instruction_set_bit_reg(m_State.C, 1); break;
			case 0xCBCA: Instruction_set_bit_reg(m_State.D, 1); break;
			case 0xCBCB: Instruction_set_bit_reg(m_State.E, 1); break;
			case 0xCBCC: Instruction_set_bit_reg(m_State.H, 1); break;
			case 0xCBCD: Instruction_set_bit_reg(m_State.L, 1); break;
			case 0xCBCE: Instruction_set_bit_hl(1); m_Cycles = 16; break;
			case 0xCBCF: Instruction_set_bit_reg(m_State.A, 1); break;
			case 0xCBD0: Instruction_set_bit_reg(m_State.B, 2); break;
			case 0xCBD1: Instruction_set_bit_reg(m_State.C, 2); break;
			case 0xCBD2: Instruction_set_bit_reg(m_State.D, 2); break;
			case 0xCBD3: Instruction_set_bit_reg(m_State.E, 2); break;
			case 0xCBD4: Instruction_set_bit_reg(m_State.H, 2); break;
			case 0xCBD5: Instruction_set_bit_reg(m_State.L, 2); break;
			case 0xCBD6: Instruction_set_bit_hl(2); m_Cycles = 16; break;
			case 0xCBD7: Instruction_set_bit_reg(m_State.A, 2); break;
			case 0xCBD8: Instruction_set_bit_reg(m_State.B, 3); break;
			case 0xCBD9: Instruction_set_bit_reg(m_State.C, 3); break;
			case 0xCBDA: Instruction_set_bit_reg(m_State.D, 3); break;
			case 0xCBDB: Instruction_set_bit_reg(m_State.E, 3); break;
			case 0xCBDC: Instruction_set_bit_reg(m_State.H, 3); break;
			case 0xCBDD: Instruction_set_bit_reg(m_State.L, 3); break;
			case 0xCBDE: Instruction_set_bit_hl(3); m_Cycles = 16; break;
			case 0xCBDF: Instruction_set_bit_reg(m_State.A, 3); break;
			case 0xCBE0: Instruction_set_bit_reg(m_State.B, 4); break;
			case 0xCBE1: Instruction_set_bit_reg(m_State.C, 4); break;
			case 0xCBE2: Instruction_set_bit_reg(m_State.D, 4); break;
			case 0xCBE3: Instruction_set_bit_reg(m_State.E, 4); break;
			case 0xCBE4: Instruction_set_bit_reg(m_State.H, 4); break;
			case 0xCBE5: Instruction_set_bit_reg(m_State.L, 4); break;
			case 0xCBE6: Instruction_set_bit_hl(4); m_Cycles = 16; break;
			case 0xCBE7: Instruction_set_bit_reg(m_State.A, 4); break;
			case 0xCBE8: Instruction_set_bit_reg(m_State.B, 5); break;
			case 0xCBE9: Instruction_set_bit_reg(m_State.C, 5); break;
			case 0xCBEA: Instruction_set_bit_reg(m_State.D, 5); break;
			case 0xCBEB: Instruction_set_bit_reg(m_State.E, 5); break;
			case 0xCBEC: Instruction_set_bit_reg(m_State.H, 5); break;
			case 0xCBED: Instruction_set_bit_reg(m_State.L, 5); break;
			case 0xCBEE: Instruction_set_bit_hl(5); m_Cycles = 16; break;
			case 0xCBEF: Instruction_set_bit_reg(m_State.A, 5); break;
			case 0xCBF0: Instruction_set_bit_reg(m_State.B, 6); break;
			case 0xCBF1: Instruction_set_bit_reg(m_State.C, 6); break;
			case 0xCBF2: Instruction_set_bit_reg(m_State.D, 6); break;
			case 0xCBF3: Instruction_set_bit_reg(m_State.E, 6); break;
			case 0xCBF4: Instruction_set_bit_reg(m_State.H, 6); break;
			case 0xCBF5: Instruction_set_bit_reg(m_State.L, 6); break;
			case 0xCBF6: Instruction_set_bit_hl(6); m_Cycles = 16; break;
			case 0xCBF7: Instruction_set_bit_reg(m_State.A, 6); break;
			case 0xCBF8: Instruction_set_bit_reg(m_State.B, 7); break;
			case 0xCBF9: Instruction_set_bit_reg(m_State.C, 7); break;
			case 0xCBFA: Instruction_set_bit_reg(m_State.D, 7); break;
			case 0xCBFB: Instruction_set_bit_reg(m_State.E, 7); break;
			case 0xCBFC: Instruction_set_bit_reg(m_State.H, 7); break;
			case 0xCBFD: Instruction_set_bit_reg(m_State.L, 7); break;
			case 0xCBFE: Instruction_set_bit_hl(7); m_Cycles = 16; break;
			case 0xCBFF: Instruction_set_bit_reg(m_State.A, 7); break;

			default:
				Logger::Instance().Error(Domain::CPU, "Error: Unimplemented instruction");
				break;
		}
	}

	std::map<uint16_t, std::string> Cpu::DisassebleAll()
	{
		std::map<uint16_t, std::string> mapLines;
		uint16_t pc = 0;

		while (pc < 0x7FFF)
		{
			uint8_t* opcode = &m_MMU.Read(pc);
			int nextPC;

			if (opcode[0] != 0xCB)
			{
				nextPC = instrSet.m_InstructionMap[opcode[0]].length;
			}
			else
			{
				nextPC = 2;
			}

			mapLines[pc] = DisassembleInstruction(opcode);
			pc += nextPC;
		}

		return mapLines;
	}

	std::string Cpu::GetCurrentInstruction()
	{
		return m_CurrentInstructionName;
	}

	void Cpu::Reset(bool enableBootRom)
	{
		Cpu::m_TotalCycles = 0;
		m_IsHalted = false;

		// registers
		m_State.AF = enableBootRom ? 0x0000 : 0x01B0; // flags - should be reset to $B0
		m_State.BC = enableBootRom ? 0x0000 : 0x0013;
		m_State.DE = enableBootRom ? 0x0000 : 0x00D8;
		m_State.HL = enableBootRom ? 0x0000 : 0x014D;
		m_State.PC = enableBootRom ? 0x0000 : 0x100; // game boy execution start point
		m_State.SP = enableBootRom ? 0x0000 : 0xFFFE;
		m_State.IME = false;

		// hardware registers
		m_MMU.Write(0xFF00, 0xCF);
		m_MMU.Write(0xFF01, 0x00);
		m_MMU.Write(0xFF02, 0x7E);
		//m_MMU.Write(0xFF04, 0xAB);
		m_MMU.Write(0xFF05, 0x00);
		m_MMU.Write(0xFF06, 0x00);
		m_MMU.Write(0xFF07, 0x00);
		m_MMU.Write(0xFF0F, 0x00);
		m_MMU.Write(0xFF10, 0x80);
		m_MMU.Write(0xFF11, 0xBF);
		m_MMU.Write(0xFF12, 0xF3);
		m_MMU.Write(0xFF13, 0xFF);
		m_MMU.Write(0xFF14, 0xBF);
		m_MMU.Write(0xFF16, 0x3F);
		m_MMU.Write(0xFF17, 0x00);
		m_MMU.Write(0xFF18, 0xFF);
		m_MMU.Write(0xFF19, 0xBF);
		m_MMU.Write(0xFF1A, 0x7F);
		m_MMU.Write(0xFF1B, 0xFF);
		m_MMU.Write(0xFF1C, 0x9F);
		m_MMU.Write(0xFF1D, 0xFF);
		m_MMU.Write(0xFF1E, 0xBF);
		m_MMU.Write(0xFF20, 0xFF);
		m_MMU.Write(0xFF21, 0x00);
		m_MMU.Write(0xFF22, 0x00);
		m_MMU.Write(0xFF23, 0xBF);
		m_MMU.Write(0xFF24, 0x77);
		m_MMU.Write(0xFF25, 0xF3);
		m_MMU.Write(0xFF26, 0xF1);
		m_MMU.Write(0xFF40, 0x91);
		m_MMU.Write(0xFF41, 0x85);
		m_MMU.Write(0xFF42, 0x00);
		m_MMU.Write(0xFF43, 0x00);
		m_MMU.Write(0xFF44, 0x00);
		m_MMU.Write(0xFF45, 0x00);
		m_MMU.Write(0xFF46, 0xFF);
		m_MMU.Write(0xFF47, 0xFC);
		m_MMU.Write(0xFF48, 0xFF);
		m_MMU.Write(0xFF49, 0xFF);
		m_MMU.Write(0xFF4A, 0x00);
		m_MMU.Write(0xFF4B, 0x00);
		m_MMU.Write(HW_FFFF_INTERRUPT_ENABLE, 0x00);
	}

	void Cpu::PushSP(uint16_t value)
	{
		m_MMU.Write(--m_State.SP, (value >> 8) & 0xFF);
		m_MMU.Write(--m_State.SP, value & 0xFF);
	}

	uint16_t Cpu::PopSP()
	{
		uint8_t firstByte = m_MMU.Read(m_State.SP++);
		uint8_t secondByte = m_MMU.Read(m_State.SP++);
		return (secondByte << 8) | (firstByte);
	}

	bool Cpu::GetCPUFlag(int flag)
	{
		return (m_State.F & flag) != 0;
	}

	void Cpu::SetCPUFlag(int flag, bool enable)
	{
		if (enable)
			m_State.F |= flag;
		else
			m_State.F &= ~flag;
	}

	void Cpu::Instruction_ld_reg_value(uint8_t& reg, uint8_t& value)
	{
		reg = value;
	}

	void Cpu::Instruction_ld_addr_reg(uint16_t& address, uint8_t& reg)
	{
		m_MMU.Write(address, reg);
	}

	void Cpu::Instruction_ld_reg_addr(uint8_t& reg, uint16_t& address)
	{
		reg = m_MMU.Read(address);
	}

	void Cpu::Instruction_ld16_reg_value(uint16_t& reg, uint16_t value)
	{
		reg = value;
	}

	void Cpu::Instruction_inc_reg(uint8_t& reg)
	{
		reg++;

		SetCPUFlag(FLAG_ZERO, (reg == 0));
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, (reg & 0x0F) == 0x00);
	}

	void Cpu::Instruction_inc_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_inc_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_dec_reg(uint8_t& reg)
	{
		reg--;

		SetCPUFlag(FLAG_ZERO, (reg == 0));
		SetCPUFlag(FLAG_SUBTRACT, true);
		SetCPUFlag(FLAG_HALF_CARRY, (reg & 0x0F) == 0x0F);
	}

	void Cpu::Instruction_dec_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_dec_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_add_reg(uint8_t& reg)
	{
		uint16_t fullResult = m_State.A + reg;
		uint8_t result = static_cast<uint8_t>(fullResult);

		// Update flags
		SetCPUFlag(FLAG_ZERO, result == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, ((m_State.A & 0x0F) + (reg & 0x0F) > 0x0F));
		SetCPUFlag(FLAG_CARRY, (fullResult > 0xFF));

		m_State.A = result;
	}

	void Cpu::Instruction_add_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_add_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_adc_reg(uint8_t& reg)
	{
		bool carry = GetCPUFlag(FLAG_CARRY);
		uint16_t fullResult = m_State.A + reg + (carry ? 1 : 0);
		uint8_t result = static_cast<uint8_t>(fullResult);

		// Update flags
		SetCPUFlag(FLAG_ZERO, result == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, (m_State.A & 0xF) + (reg & 0xF) + (carry ? 1 : 0) > 0xF);
		SetCPUFlag(FLAG_CARRY, (fullResult > 0xFF));

		m_State.A = result;
	}

	void Cpu::Instruction_adc_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_adc_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_sub_reg(uint8_t& reg)
	{
		uint8_t result = m_State.A - reg;

		// Update flags
		SetCPUFlag(FLAG_ZERO, result == 0);
		SetCPUFlag(FLAG_SUBTRACT, true);
		SetCPUFlag(FLAG_HALF_CARRY, (m_State.A & 0x0F) - (reg & 0x0F) < 0);
		SetCPUFlag(FLAG_CARRY, m_State.A < reg);

		m_State.A = result;
	}

	void Cpu::Instruction_sub_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_sub_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_sbc_reg(uint8_t& reg)
	{
		bool carry = GetCPUFlag(FLAG_CARRY);
		int32_t fullResult = m_State.A - reg - (carry ? 1 : 0);
		uint8_t result = static_cast<uint8_t>(fullResult);

		SetCPUFlag(FLAG_ZERO, result == 0);
		SetCPUFlag(FLAG_SUBTRACT, true);
		SetCPUFlag(FLAG_HALF_CARRY, (m_State.A & 0xF) - (reg & 0xF) - carry < 0);
		SetCPUFlag(FLAG_CARRY, fullResult < 0);

		m_State.A = result;
	}

	void Cpu::Instruction_sbc_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_sbc_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_and_reg(uint8_t& reg)
	{
		m_State.A = m_State.A & reg;

		SetCPUFlag(FLAG_ZERO, m_State.A == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, true);
		SetCPUFlag(FLAG_CARRY, false);
	}

	void Cpu::Instruction_and_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_and_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_xor_reg(uint8_t& reg)
	{
		m_State.A = m_State.A ^ reg;

		SetCPUFlag(FLAG_ZERO, m_State.A == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
		SetCPUFlag(FLAG_CARRY, false);
	}

	void Cpu::Instruction_xor_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_xor_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_or_reg(uint8_t& reg)
	{
		m_State.A = m_State.A | reg;

		SetCPUFlag(FLAG_ZERO, m_State.A == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
		SetCPUFlag(FLAG_CARRY, false);
	}

	void Cpu::Instruction_or_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_or_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_cp_reg(uint8_t& reg)
	{
		SetCPUFlag(FLAG_ZERO, m_State.A == reg);
		SetCPUFlag(FLAG_SUBTRACT, true);
		SetCPUFlag(FLAG_HALF_CARRY, (m_State.A & 0xF) - (reg & 0xF) < 0);
		SetCPUFlag(FLAG_CARRY, m_State.A < reg);
	}

	void Cpu::Instruction_cp_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_cp_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_inc_reg16(uint16_t& reg)
	{
		reg++;
	}

	void Cpu::Instruction_dec_reg16(uint16_t& reg)
	{
		reg--;
	}

	void Cpu::Instruction_add_reg16(uint16_t& reg)
	{
		uint32_t fullResult = m_State.HL + reg;
		uint16_t result = static_cast<uint16_t>(fullResult);

		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, (m_State.HL ^ reg ^ (fullResult & 0xFFFF)) & 0x1000);
		SetCPUFlag(FLAG_CARRY, fullResult > 0xFFFF);

		m_State.HL = result;
	}

	void Cpu::Instruction_add_sp_e8(uint8_t& e8)
	{
		int8_t offset = e8;
		int32_t fullResult = m_State.SP + offset;
		uint16_t result = static_cast<uint16_t>(fullResult);

		SetCPUFlag(FLAG_ZERO, false);
		SetCPUFlag(FLAG_SUBTRACT, false);

		SetCPUFlag(FLAG_HALF_CARRY, ((m_State.SP ^ offset ^ (fullResult & 0xFFFF)) & 0x10) == 0x10);
		SetCPUFlag(FLAG_CARRY, ((m_State.SP ^ offset ^ (fullResult & 0xFFFF)) & 0x100) == 0x100);

		m_State.SP = result;
	}

	void Cpu::Instruction_rlc_reg(uint8_t& reg)
	{
		uint8_t result = (reg << 1) | ((reg >> 7) & 0x1);

		SetCPUFlag(FLAG_ZERO, result == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
		SetCPUFlag(FLAG_CARRY, ((reg >> 7) & 0x1) != 0);

		reg = result;
	}

	void Cpu::Instruction_rlc_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_rlc_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_rrc_reg(uint8_t& reg)
	{
		SetCPUFlag(FLAG_CARRY, (reg & 0x01) != 0);

		reg = (reg >> 1) | (GetCPUFlag(FLAG_CARRY) ? 0x80 : 0x00);

		SetCPUFlag(FLAG_ZERO, (reg == 0));
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
	}

	void Cpu::Instruction_rrc_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_rrc_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_rl_reg(uint8_t& reg)
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

	void Cpu::Instruction_rl_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_rl_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_rr_reg(uint8_t& reg)
	{
		uint8_t carry = GetCPUFlag(FLAG_CARRY) ? 0x80 : 0;
		uint8_t result = carry | (reg >> 1);

		SetCPUFlag(FLAG_ZERO, result == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
		SetCPUFlag(FLAG_CARRY, (reg & 0x1) != 0);

		reg = result;
	}

	void Cpu::Instruction_rr_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_rr_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_sla_reg(uint8_t& reg)
	{
		SetCPUFlag(FLAG_CARRY, (reg & 0x80) != 0);

		reg = reg << 1;

		SetCPUFlag(FLAG_ZERO, (reg == 0));
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
	}

	void Cpu::Instruction_sla_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_sla_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_sra_reg(uint8_t& reg)
	{
		uint8_t result = (reg & 0x80) | (reg >> 1);

		SetCPUFlag(FLAG_ZERO, result == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
		SetCPUFlag(FLAG_CARRY, ((reg & 0x1) != 0));

		reg = result;
	}

	void Cpu::Instruction_sra_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_sra_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_swap_reg(uint8_t& reg)
	{
		reg = ((reg & 0x0F) << 4) | ((reg & 0xF0) >> 4);

		SetCPUFlag(FLAG_ZERO, (reg == 0));
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
		SetCPUFlag(FLAG_CARRY, false);
	}

	void Cpu::Instruction_swap_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_swap_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_srl_reg(uint8_t& reg)
	{
		SetCPUFlag(FLAG_CARRY, (reg & 0x01) != 0);

		reg = reg >> 1;

		SetCPUFlag(FLAG_ZERO, (reg == 0));
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
	}

	void Cpu::Instruction_srl_hl()
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_srl_reg(value);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_bit_bit_reg(uint8_t& reg, uint8_t bit)
	{
		SetCPUFlag(FLAG_ZERO, (reg & (1 << bit)) == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, true);
	}

	void Cpu::Instruction_bit_bit_hl(uint8_t bit)
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_bit_bit_reg(value, bit);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_res_bit_reg(uint8_t& reg, uint8_t bit)
	{
		reg &= ~(1 << bit);
	}

	void Cpu::Instruction_res_bit_hl(uint8_t bit)
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_res_bit_reg(value, bit);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::Instruction_set_bit_reg(uint8_t& reg, uint8_t bit)
	{
		reg |= (1 << bit);
	}

	void Cpu::Instruction_set_bit_hl(uint8_t bit)
	{
		uint8_t value = m_MMU.Read(m_State.HL);
		Instruction_set_bit_reg(value, bit);
		m_MMU.Write(m_State.HL, value);
	}

	void Cpu::ProcessTimers() 
	{
		// increment DIV register
		uint16_t internalClock = (m_MMU.Read(HW_FF04_DIV_DIVIDER_REGISTER) << 8)
			| m_MMU.Read(HW_FF03_DIV_DIVIDER_REGISTER_LOW);

		internalClock++;

		// if the div clock rolls over then we need to copy the value of TIMA to TMA
		if (internalClock == 0xFFFF)
		{
			m_MMU.Write(HW_FF06_TMA_TIMER_MODULO, m_MMU.Read(HW_FF05_TIMA_TIMER_COUNTER));
		}

		// write updated DIV register
		m_MMU.Write(HW_FF04_DIV_DIVIDER_REGISTER, (internalClock & 0xFF00) >> 8, true);
		m_MMU.Write(HW_FF03_DIV_DIVIDER_REGISTER_LOW, internalClock & 0x00FF, true);


		// https://github.com/Hacktix/GBEDG/blob/master/timers/index.md#timer-operation
		static bool lastBit;
		bool thisBit = 0;

		// 1. A bit position of the 16 - bit counter is determined based on the lower 2 bits of the TAC register
		switch (m_MMU.Read(HW_FF07_TAC_TIMER_CONTROL) & 0x03)
		{
			case 0:
				thisBit = (internalClock >> 9) & 0x1;
				break;
			case 1:
				thisBit = (internalClock >> 3) & 0x1;
				break;
			case 2:
				thisBit = (internalClock >> 5) & 0x1;
				break;
			case 3:
				thisBit = (internalClock >> 7) & 0x1;
				break;
		}

		// 2. The "Timer Enable" bit(Bit 2) is extracted from the value in the TAC register and stored for the next step.
		bool timerEnabled = m_MMU.ReadRegisterBit(HW_FF07_TAC_TIMER_CONTROL, TAC_ENABLE);

		// 3. The bit taken from the DIV counter is ANDed with the Timer Enable bit. 
		//    The result of this operation will be referred to as the "AND Result".
		thisBit &= timerEnabled;

		static int countdownToInterrupt = 0;
		if (lastBit == 1 && thisBit == 0)
		{
			// now increment the TIMA register
			uint8_t tima = m_MMU.Read(HW_FF05_TIMA_TIMER_COUNTER);
			tima++;
			m_MMU.Write(HW_FF05_TIMA_TIMER_COUNTER, tima);

			// if the TIMA register rolls over then we need to trigger an interrupt
			if (tima == 0x0)
			{
				// need to wait four cycles plus this current cycle
				countdownToInterrupt = 5;
			}
		}

		if (countdownToInterrupt > 0)
		{
			countdownToInterrupt--;
			if (countdownToInterrupt == 0)
			{
				m_MMU.WriteRegisterBit(HW_FF0F_IF_INTERRUPT_FLAG, IF_TIMER, true);
				m_MMU.Write(HW_FF05_TIMA_TIMER_COUNTER, m_MMU.Read(HW_FF06_TMA_TIMER_MODULO));
			}
		}

		lastBit = thisBit;
	}

	Cpu::sCPUState* Cpu::GetState()
	{
		return &m_State;
	}

	void Cpu::SetState(sCPUState state)
	{
		m_State = state;
	}

	IMmu& Cpu::GetMMU()
	{
		return m_MMU;
	}

	std::string Cpu::DisassembleInstruction(uint8_t* opcode)
	{
		uint8_t numOfBytes = instrSet.m_InstructionMap[opcode[0]].length;
		std::ostringstream instrBuffer;

		instrBuffer << std::hex << std::setfill('0') << std::uppercase <<
			"A:" << std::setw(2) << static_cast<int>(m_State.A) <<
			" F:" << (GetCPUFlag(FLAG_ZERO) ? "Z" : "-") <<
			(GetCPUFlag(FLAG_SUBTRACT) ? "N" : "-") <<
			(GetCPUFlag(FLAG_HALF_CARRY) ? "H" : "-") <<
			(GetCPUFlag(FLAG_CARRY) ? "C" : "-") <<
			" BC:" << std::setw(4) << static_cast<int>(m_State.BC) << std::nouppercase <<
			" DE:" << std::setw(4) << static_cast<int>(m_State.DE) <<
			" HL:" << std::setw(4) << static_cast<int>(m_State.HL) <<
			" SP:" << std::setw(4) << static_cast<int>(m_State.SP) <<
			" PC:" << std::setw(4) << static_cast<int>(m_State.PC) <<
			" (cy: " << std::dec << static_cast<int>(m_TotalCycles) - 1 << ")" << std::hex <<
			" ppu:+" << static_cast<int>(m_MMU.Read(HW_FF41_STAT_LCD_STATUS) & 0b11) <<
			" |[00]0x" << std::setw(4) << static_cast<int>(m_State.PC) << ":" <<
			std::setw(2) << static_cast<int>(opcode[0]) << " ";

		if (numOfBytes > 1)
		{
			instrBuffer << std::setw(2) << static_cast<int>(opcode[1]) << " ";
		}
		else
		{
			instrBuffer << "   ";
		}

		if (numOfBytes > 2)
		{
			instrBuffer << std::setw(2) << static_cast<int>(opcode[2]) << "";
		}
		else
		{
			instrBuffer << "   ";
		}

		if (opcode[0] != 0xCB)
		{
			instrBuffer << "  " << instrSet.m_InstructionMap[opcode[0]].nmemonic;
		}
		else
		{
			instrBuffer << "  " << instrSet.m_16BitInstructionMap[opcode[1]].nmemonic;
		}

		instrBuffer << std::endl;

		return instrBuffer.str();
	}
}