#include "Cpu.h"
#include "Defines.h"
#include "Logger.h"
#include "Utils.h"

#include <fstream>
#include <filesystem>
#include <sstream>

namespace Core
{
	Cpu::Cpu(Mmu& mmu) : m_MMU(mmu)
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

		linecount++;

		// read opcode from memory
		uint8_t* opcode = &m_MMU.Read(State.PC);

		//Disassemble(opcode, State.PC);

		if (false)
		{
			logBuffer << std::hex << std::setfill('0') << std::uppercase <<
				"A:" << std::setw(2) << static_cast<int>(State.A) <<
				" F:" << (GetCPUFlag(FLAG_ZERO) ? "Z" : "-") <<
					     (GetCPUFlag(FLAG_SUBTRACT) ? "N" : "-") << 
					     (GetCPUFlag(FLAG_HALF_CARRY) ? "H" : "-") <<
					     (GetCPUFlag(FLAG_CARRY) ? "C" : "-") <<
				" BC:" << std::setw(4) << static_cast<int>(State.BC) << std::nouppercase <<
				" DE:" << std::setw(4) << static_cast<int>(State.DE) <<
				" HL:" << std::setw(4) << static_cast<int>(State.HL) <<
				" SP:" << std::setw(4) << static_cast<int>(State.SP) <<
				" PC:" << std::setw(4) << static_cast<int>(State.PC) <<
				" (cy: " << std::dec << static_cast<int>(m_TotalCycles) - 1 << ")" << std::hex <<
				" ppu:+" << static_cast<int>(m_MMU.Read(HW_STAT_LCD_STATUS) & 0b11) <<
				" |[00]0x" << std::setw(4) << static_cast<int>(State.PC) << ":" <<
				std::setw(2) << static_cast<int>(opcode[0]) << " "
				<< std::setw(2) << static_cast<int>(opcode[1]) << " "
				<< std::setw(2) << static_cast<int>(opcode[2]) << "" << std::endl;

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
			State.PC++;

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
					State.PC++;
					m_Cycles = 4;
					m_MMU.ResetDIVTimer();
					break;
				}

					// "HALT" B:1 C:4 FLAGS: - - - -
				case 0x76: m_IsHalted = true; m_Cycles = 4;	break;

					// "PREFIX" B:1 C:4 FLAGS: - - - -
				case 0xCB: Process16bitInstruction((opcode[0] << 8) | (opcode[1]), State); State.PC++; break;

					// "DI" B:1 C:4 FLAGS: - - - -
				case 0xF3: m_InterruptMasterFlag = false; m_Cycles = 4; break;

					// "EI" B:1 C:4 FLAGS: - - - -
				case 0xFB: m_InterruptMasterFlag = true; m_Cycles = 4; break;


					/********************************************************************************************
						Jumps/Calls
					*********************************************************************************************/

					// "JR e8" B:2 C:12 FLAGS: - - - -
				case 0x18:
				{
					State.PC += (int8_t)opcode[1] + 1;

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
						State.PC += offset + 1;
						m_Cycles = 12;
					}
					else
					{
						State.PC++;
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
						State.PC += offset + 1;
						m_Cycles = 12;
					}
					else
					{
						State.PC++;
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
						State.PC += offset + 1;
						m_Cycles = 12;
					}
					else
					{
						State.PC++;
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
						State.PC += offset + 1;
						m_Cycles = 12;
					}
					else
					{
						State.PC++;
						m_Cycles = 8;
					}

					break;
				}

				// "RET NZ" B:1 C:20/8 FLAGS: - - - -
				case 0xC0:
				{
					if (!GetCPUFlag(FLAG_ZERO))
					{
						State.PC = PopSP();
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
						State.PC = offset;

						m_Cycles = 16;
						break;
					}

					State.PC += 2;

					m_Cycles = 12;
					break;
				}

						 // "JP a16" B:3 C:16 FLAGS: - - - -
				case 0xC3:
				{
					uint16_t offset = (opcode[2] << 8) | (opcode[1]);
					State.PC = offset;

					m_Cycles = 16;
					break;
				}

				// "CALL NZ a16" B:3 C:24/12 FLAGS: - - - -
				case 0xC4:
				{
					if (!GetCPUFlag(FLAG_ZERO))
					{
						PushSP(State.PC += 2);
						State.PC = (opcode[2] << 8) | (opcode[1]);

						m_Cycles = 24;
						break;
					}

					State.PC += 2;

					m_Cycles = 12;
					break;
				}

				// "RST $00" B:1 C:16 FLAGS: - - - -
				case 0xC7:
				{
					PushSP(State.PC);
					State.PC = 0x00;

					m_Cycles = 16;
					break;
				}

				// "RET Z" B:1 C:20/8 FLAGS: - - - -
				case 0xC8:
				{
					if (GetCPUFlag(FLAG_ZERO))
					{
						State.PC = PopSP();
						m_Cycles = 20;
						break;
					}

					m_Cycles = 8;
					break;
				}

				// "RET" B:1 C:16 FLAGS: - - - -
				case 0xC9:
				{
					State.PC = PopSP();

					m_Cycles = 16;
					break;
				}

				// "JP Z a16" B:3 C:16/12 FLAGS: - - - -
				case 0xCA:
				{
					if (GetCPUFlag(FLAG_ZERO))
					{
						uint16_t offset = (opcode[2] << 8) | (opcode[1]);
						State.PC = offset;

						m_Cycles = 16;
						break;
					}

					State.PC += 2;

					m_Cycles = 12;
					break;
				}

				// "CALL Z a16" B:3 C:24/12 FLAGS: - - - -
				case 0xCC:
				{
					if (GetCPUFlag(FLAG_ZERO))
					{
						PushSP(State.PC += 2);
						State.PC = (opcode[2] << 8) | (opcode[1]);
						m_Cycles = 24;
						break;
					}

					State.PC += 2;

					m_Cycles = 12;
					break;
				}

				// "CALL a16" B:3 C:24 FLAGS: - - - -
				case 0xCD:
				{
					PushSP(State.PC += 2);
					State.PC = (opcode[2] << 8) | (opcode[1]);

					m_Cycles = 24;
					break;
				}

				// "RST $08" B:1 C:16 FLAGS: - - - -
				case 0xCF:
				{
					PushSP(State.PC);
					State.PC = 0x08;

					m_Cycles = 16;
					break;
				}

				// "RET NC" B:1 C:20/8 FLAGS: - - - -
				case 0xD0:
				{
					if (!GetCPUFlag(FLAG_CARRY))
					{
						State.PC = PopSP();
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
						State.PC = offset;

						m_Cycles = 16;
						break;
					}

					State.PC += 2;

					m_Cycles = 12;
					break;
				}

				// "CALL NC a16" B:3 C:24/12 FLAGS: - - - -
				case 0xD4:
				{
					if (!GetCPUFlag(FLAG_CARRY))
					{
						PushSP(State.PC += 2);
						State.PC = (opcode[2] << 8) | (opcode[1]);

						m_Cycles = 24;
						break;
					}

					State.PC += 2;

					m_Cycles = 12;
					break;
				}

				// "RST $10" B:1 C:16 FLAGS: - - - -
				case 0xD7:
				{
					PushSP(State.PC);
					State.PC = 0x10;

					m_Cycles = 16;
					break;
				}

				// "RET C" B:1 C:20/8 FLAGS: - - - -
				case 0xD8:
				{
					if (GetCPUFlag(FLAG_CARRY))
					{
						State.PC = PopSP();
						m_Cycles = 20;
						break;
					}

					m_Cycles = 8;
					break;
				}

				// "RETI" B:1 C:16 FLAGS: - - - -
				case 0xD9:
				{
					State.PC = PopSP();
					m_InterruptMasterFlag = true;

					m_Cycles = 16;
					break;
				}

				// "JP C a16" B:3 C:16/12 FLAGS: - - - -
				case 0xDA:
				{
					if (GetCPUFlag(FLAG_CARRY))
					{
						uint16_t offset = (opcode[2] << 8) | (opcode[1]);
						State.PC = offset;

						m_Cycles = 16;
						break;
					}

					State.PC += 2;

					m_Cycles = 12;
					break;
				}

						 // "CALL C a16" B:3 C:24/12 FLAGS: - - - -
				case 0xDC:
				{
					if (GetCPUFlag(FLAG_CARRY))
					{
						PushSP(State.PC += 2);
						State.PC = (opcode[2] << 8) | (opcode[1]);

						m_Cycles = 24;
						break;
					}

					State.PC += 2;

					m_Cycles = 12;
					break;
				}

						 // "RST $18" B:1 C:16 FLAGS: - - - -
				case 0xDF:
				{
					PushSP(State.PC);
					State.PC = 0x18;

					m_Cycles = 16;
					break;
				}

				// "RST $20" B:1 C:16 FLAGS: - - - -
				case 0xE7:
				{
					PushSP(State.PC);
					State.PC = 0x20;

					m_Cycles = 16;
					break;
				}

				// "JP HL" B:1 C:4 FLAGS: - - - -
				case 0xE9:
				{
					State.PC = State.HL;

					m_Cycles = 4;
					break;
				}

				// "RST $28" B:1 C:16 FLAGS: - - - -
				case 0xEF:
				{
					PushSP(State.PC);
					State.PC = 0x28;

					m_Cycles = 16;
					break;
				}


				// "RST $30" B:1 C:16 FLAGS: - - - -
				case 0xF7:
				{
					PushSP(State.PC);
					State.PC = 0x30;

					m_Cycles = 16;
					break;
				}


				// "RST $38" B:1 C:16 FLAGS: - - - -
				case 0xFF:
				{
					PushSP(State.PC);
					State.PC = 0x38;

					m_Cycles = 16;
					break;
				}


				/********************************************************************************************
					8-bit Load Instructions
				*********************************************************************************************/

				// "LD [addr] reg" B:1 C:8 FLAGS: - - - -
				case 0x02: Instruction_ld_addr_reg(State.BC, State.A); m_Cycles = 8; break;
				case 0x12: Instruction_ld_addr_reg(State.DE, State.A); m_Cycles = 8; break;

					// "LD reg [addr]" B:1 C:8 FLAGS: - - - -
				case 0x0A: Instruction_ld_reg_addr(State.A, State.BC); m_Cycles = 8; break;
				case 0x1A: Instruction_ld_reg_addr(State.A, State.DE); m_Cycles = 8; break;
				case 0x46: Instruction_ld_reg_addr(State.B, State.HL); m_Cycles = 8; break;
				case 0x4E: Instruction_ld_reg_addr(State.C, State.HL); m_Cycles = 8; break;
				case 0x56: Instruction_ld_reg_addr(State.D, State.HL); m_Cycles = 8; break;
				case 0x5E: Instruction_ld_reg_addr(State.E, State.HL); m_Cycles = 8; break;
				case 0x66: Instruction_ld_reg_addr(State.H, State.HL); m_Cycles = 8; break;
				case 0x6E: Instruction_ld_reg_addr(State.L, State.HL); m_Cycles = 8; break;
				case 0x7E: Instruction_ld_reg_addr(State.A, State.HL); m_Cycles = 8; break;

					// "LD reg n8" B:2 C:8 FLAGS: - - - -
				case 0x06: Instruction_ld_reg_value(State.B, opcode[1]); State.PC++; m_Cycles = 8; break;
				case 0x0E: Instruction_ld_reg_value(State.C, opcode[1]); State.PC++; m_Cycles = 8; break;
				case 0x16: Instruction_ld_reg_value(State.D, opcode[1]); State.PC++; m_Cycles = 8; break;
				case 0x1E: Instruction_ld_reg_value(State.E, opcode[1]); State.PC++; m_Cycles = 8; break;
				case 0x26: Instruction_ld_reg_value(State.H, opcode[1]); State.PC++; m_Cycles = 8; break;
				case 0x2E: Instruction_ld_reg_value(State.L, opcode[1]); State.PC++; m_Cycles = 8; break;


					// "LD A [HL-]" B:1 C:8 FLAGS: - - - -
				case 0x3A:
				{
					State.A = m_MMU.Read(State.HL);
					State.HL--;

					m_Cycles = 8;
					break;
				}

				// "LD [HL+] A" B:1 C:8 FLAGS: - - - -
				case 0x22:
				{
					m_MMU.Write(State.HL, State.A);
					State.HL++;

					m_Cycles = 8;
					break;
				}

				// "LD A [HL+]" B:1 C:8 FLAGS: - - - -
				case 0x2A:
				{
					State.A = m_MMU.Read(State.HL);
					State.HL++;

					m_Cycles = 8;
					break;
				}

				// "LD [HL-] A" B:1 C:8 FLAGS: - - - -
				case 0x32:
				{
					m_MMU.Write(State.HL, State.A);
					State.HL--;

					m_Cycles = 8;
					break;
				}

				// "LD [HL] n8" B:2 C:12 FLAGS: - - - -
				case 0x36:
				{
					m_MMU.Write(State.HL, opcode[1]);
					State.PC++;

					m_Cycles = 12;
					break;
				}

				// "LD A n8" B:2 C:8 FLAGS: - - - -
				case 0x3E:
				{
					State.A = opcode[1];
					State.PC++;

					m_Cycles = 8;
					break;
				}

				// "LD reg reg" B:1 C:4 FLAGS: - - - -
				case 0x40: Instruction_ld_reg_value(State.B, State.B); m_Cycles = 4; break;
				case 0x41: Instruction_ld_reg_value(State.B, State.C); m_Cycles = 4; break;
				case 0x42: Instruction_ld_reg_value(State.B, State.D); m_Cycles = 4; break;
				case 0x43: Instruction_ld_reg_value(State.B, State.E); m_Cycles = 4; break;
				case 0x44: Instruction_ld_reg_value(State.B, State.H); m_Cycles = 4; break;
				case 0x45: Instruction_ld_reg_value(State.B, State.L); m_Cycles = 4; break;
				case 0x47: Instruction_ld_reg_value(State.B, State.A); m_Cycles = 4; break;

				case 0x48: Instruction_ld_reg_value(State.C, State.B); m_Cycles = 4; break;
				case 0x49: Instruction_ld_reg_value(State.C, State.C); m_Cycles = 4; break;
				case 0x4A: Instruction_ld_reg_value(State.C, State.D); m_Cycles = 4; break;
				case 0x4B: Instruction_ld_reg_value(State.C, State.E); m_Cycles = 4; break;
				case 0x4C: Instruction_ld_reg_value(State.C, State.H); m_Cycles = 4; break;
				case 0x4D: Instruction_ld_reg_value(State.C, State.L); m_Cycles = 4; break;
				case 0x4F: Instruction_ld_reg_value(State.C, State.A); m_Cycles = 4; break;

				case 0x50: Instruction_ld_reg_value(State.D, State.B); m_Cycles = 4; break;
				case 0x51: Instruction_ld_reg_value(State.D, State.C); m_Cycles = 4; break;
				case 0x52: Instruction_ld_reg_value(State.D, State.D); m_Cycles = 4; break;
				case 0x53: Instruction_ld_reg_value(State.D, State.E); m_Cycles = 4; break;
				case 0x54: Instruction_ld_reg_value(State.D, State.H); m_Cycles = 4; break;
				case 0x55: Instruction_ld_reg_value(State.D, State.L); m_Cycles = 4; break;
				case 0x57: Instruction_ld_reg_value(State.D, State.A); m_Cycles = 4; break;

				case 0x58: Instruction_ld_reg_value(State.E, State.B); m_Cycles = 4; break;
				case 0x59: Instruction_ld_reg_value(State.E, State.C); m_Cycles = 4; break;
				case 0x5A: Instruction_ld_reg_value(State.E, State.D); m_Cycles = 4; break;
				case 0x5B: Instruction_ld_reg_value(State.E, State.E); m_Cycles = 4; break;
				case 0x5C: Instruction_ld_reg_value(State.E, State.H); m_Cycles = 4; break;
				case 0x5D: Instruction_ld_reg_value(State.E, State.L); m_Cycles = 4; break;
				case 0x5F: Instruction_ld_reg_value(State.E, State.A); m_Cycles = 4; break;

				case 0x60: Instruction_ld_reg_value(State.H, State.B); m_Cycles = 4; break;
				case 0x61: Instruction_ld_reg_value(State.H, State.C); m_Cycles = 4; break;
				case 0x62: Instruction_ld_reg_value(State.H, State.D); m_Cycles = 4; break;
				case 0x63: Instruction_ld_reg_value(State.H, State.E); m_Cycles = 4; break;
				case 0x64: Instruction_ld_reg_value(State.H, State.H); m_Cycles = 4; break;
				case 0x65: Instruction_ld_reg_value(State.H, State.L); m_Cycles = 4; break;
				case 0x67: Instruction_ld_reg_value(State.H, State.A); m_Cycles = 4; break;

				case 0x68: Instruction_ld_reg_value(State.L, State.B); m_Cycles = 4; break;
				case 0x69: Instruction_ld_reg_value(State.L, State.C); m_Cycles = 4; break;
				case 0x6A: Instruction_ld_reg_value(State.L, State.D); m_Cycles = 4; break;
				case 0x6B: Instruction_ld_reg_value(State.L, State.E); m_Cycles = 4; break;
				case 0x6C: Instruction_ld_reg_value(State.L, State.H); m_Cycles = 4; break;
				case 0x6D: Instruction_ld_reg_value(State.L, State.L); m_Cycles = 4; break;
				case 0x6F: Instruction_ld_reg_value(State.L, State.A); m_Cycles = 4; break;

				case 0x78: Instruction_ld_reg_value(State.A, State.B); m_Cycles = 4; break;
				case 0x79: Instruction_ld_reg_value(State.A, State.C); m_Cycles = 4; break;
				case 0x7A: Instruction_ld_reg_value(State.A, State.D); m_Cycles = 4; break;
				case 0x7B: Instruction_ld_reg_value(State.A, State.E); m_Cycles = 4; break;
				case 0x7C: Instruction_ld_reg_value(State.A, State.H); m_Cycles = 4; break;
				case 0x7D: Instruction_ld_reg_value(State.A, State.L); m_Cycles = 4; break;
				case 0x7F: Instruction_ld_reg_value(State.A, State.A); m_Cycles = 4; break;

					// "LD [HL] reg" B:1 C:8 FLAGS: - - - -
				case 0x70: Instruction_ld_addr_reg(State.HL, State.B); m_Cycles = 8; break;
				case 0x71: Instruction_ld_addr_reg(State.HL, State.C); m_Cycles = 8; break;
				case 0x72: Instruction_ld_addr_reg(State.HL, State.D); m_Cycles = 8; break;
				case 0x73: Instruction_ld_addr_reg(State.HL, State.E); m_Cycles = 8; break;
				case 0x74: Instruction_ld_addr_reg(State.HL, State.H); m_Cycles = 8; break;
				case 0x75: Instruction_ld_addr_reg(State.HL, State.L); m_Cycles = 8; break;
				case 0x77: Instruction_ld_addr_reg(State.HL, State.A); m_Cycles = 8; break;



					// "LDH [a8] A" B:2 C:12 FLAGS: - - - -
				case 0xE0:
				{
					uint8_t offset = opcode[1];
					m_MMU.Write(0xFF00 + offset, State.A);
					State.PC++;

					m_Cycles = 12;
					break;
				}

				// "LD [C] A" B:1 C:8 FLAGS: - - - -
				case 0xE2:
				{
					m_MMU.Write(0xFF00 + State.C, State.A);

					m_Cycles = 8;
					break;
				}

				// "LD [a16] A" B:3 C:16 FLAGS: - - - -
				case 0xEA:
				{
					m_MMU.Write((opcode[2] << 8) | (opcode[1]), State.A);
					State.PC += 2;

					m_Cycles = 16;
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
						State.A = m_MMU.Read(0xFF00 + opcode[1]);

					State.PC++;

					m_Cycles = 12;
					break;
				}

				// "LD A [C]" B:1 C:8 FLAGS: - - - -
				case 0xF2:
				{
					State.A = m_MMU.Read(0xFF00 + State.C);

					m_Cycles = 8;
					break;
				}

				// "LD A [a16]" B:3 C:16 FLAGS: - - - -
				case 0xFA:
				{
					State.A = m_MMU.Read((opcode[2] << 8) | (opcode[1]));
					State.PC += 2;

					m_Cycles = 16;
					break;
				}


				/********************************************************************************************
					16-bit Load Instructions
				*********************************************************************************************/

				// "LD reg value" B:3 C:12 FLAGS: - - - -
				case 0x01: Instruction_ld16_reg_value(State.BC, (opcode[2] << 8) | (opcode[1])); State.PC += 2; m_Cycles = 12; break;
				case 0x11: Instruction_ld16_reg_value(State.DE, (opcode[2] << 8) | (opcode[1])); State.PC += 2; m_Cycles = 12; break;
				case 0x21: Instruction_ld16_reg_value(State.HL, (opcode[2] << 8) | (opcode[1])); State.PC += 2; m_Cycles = 12; break;
				case 0x31: Instruction_ld16_reg_value(State.SP, (opcode[2] << 8) | (opcode[1])); State.PC += 2; m_Cycles = 12; break;

					// "LD [a16] SP" B:3 C:20 FLAGS: - - - -
				case 0x08:
				{
					uint16_t addr = (opcode[2] << 8) | (opcode[1]);
					m_MMU.Write(addr, State.SP & 0x00FF);
					m_MMU.Write(addr + 1, (State.SP & 0xFF00) >> 8);
					State.PC += 2;

					m_Cycles = 20;
					break;
				}

				// "POP BC" B:1 C:12 FLAGS: - - - -
				case 0xC1:
				{
					State.BC = PopSP();

					m_Cycles = 12;
					break;
				}

				// "PUSH BC" B:1 C:16 FLAGS: - - - -
				case 0xC5:
				{
					PushSP(State.BC);

					m_Cycles = 16;
					break;
				}

				// "POP DE" B:1 C:12 FLAGS: - - - -
				case 0xD1:
				{
					State.DE = PopSP();

					m_Cycles = 12;
					break;
				}

				// "PUSH DE" B:1 C:16 FLAGS: - - - -
				case 0xD5:
				{
					PushSP(State.DE);

					m_Cycles = 16;
					break;
				}

				// "POP HL" B:1 C:12 FLAGS: - - - -
				case 0xE1:
				{
					State.HL = PopSP();

					m_Cycles = 12;
					break;
				}

				// "PUSH HL" B:1 C:16 FLAGS: - - - -
				case 0xE5:
				{
					PushSP(State.HL);

					m_Cycles = 16;
					break;
				}

				// "POP AF" B:1 C:12 FLAGS: Z N H C
				case 0xF1:
				{
					State.AF = (PopSP() & 0xFFF0);

					m_Cycles = 12;
					break;
				}

				// "PUSH AF" B:1 C:16 FLAGS: - - - -
				case 0xF5:
				{
					PushSP(State.AF);

					m_Cycles = 16;
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

					m_Cycles = 12;
					break;
				}

				// "LD SP HL" B:1 C:8 FLAGS: - - - -
				case 0xF9:
				{
					State.SP = State.HL;

					m_Cycles = 8;
					break;
				}


				/********************************************************************************************
					8-bit Arithmetic/Logical Instructions
				*********************************************************************************************/

				// "INC reg" B:1 C:4 FLAGS: Z 0 H -
				case 0x04: Instruction_inc_reg(State.B); m_Cycles = 4; break;
				case 0x0C: Instruction_inc_reg(State.C); m_Cycles = 4; break;
				case 0x14: Instruction_inc_reg(State.D); m_Cycles = 4; break;
				case 0x1C: Instruction_inc_reg(State.E); m_Cycles = 4; break;
				case 0x24: Instruction_inc_reg(State.H); m_Cycles = 4; break;
				case 0x2C: Instruction_inc_reg(State.L); m_Cycles = 4; break;
				case 0x34: Instruction_inc_hl(); m_Cycles = 12; break;
				case 0x3C: Instruction_inc_reg(State.A); m_Cycles = 4; break;

					// "DEC reg" B:1 C:4 FLAGS: Z 1 H -
				case 0x05: Instruction_dec_reg(State.B); m_Cycles = 4; break;
				case 0x0D: Instruction_dec_reg(State.C); m_Cycles = 4; break;
				case 0x15: Instruction_dec_reg(State.D); m_Cycles = 4; break;
				case 0x1D: Instruction_dec_reg(State.E); m_Cycles = 4; break;
				case 0x25: Instruction_dec_reg(State.H); m_Cycles = 4; break;
				case 0x2D: Instruction_dec_reg(State.L); m_Cycles = 4; break;
				case 0x35: Instruction_dec_hl(); m_Cycles = 12; break;
				case 0x3D: Instruction_dec_reg(State.A); m_Cycles = 4; break;

					// "ADD A reg" B:1 C:4 FLAGS: Z 0 H C
				case 0x80: Instruction_add_reg(State.B); m_Cycles = 4; break;
				case 0x81: Instruction_add_reg(State.C); m_Cycles = 4; break;
				case 0x82: Instruction_add_reg(State.D); m_Cycles = 4; break;
				case 0x83: Instruction_add_reg(State.E); m_Cycles = 4; break;
				case 0x84: Instruction_add_reg(State.H); m_Cycles = 4; break;
				case 0x85: Instruction_add_reg(State.L); m_Cycles = 4; break;
				case 0x86: Instruction_add_hl(); m_Cycles = 8; break;
				case 0x87: Instruction_add_reg(State.A); m_Cycles = 4; break;
				case 0xC6: Instruction_add_reg(opcode[1]); State.PC++; m_Cycles = 8; break;

					// "ADC A reg" B:1 C:4 FLAGS: Z 0 H C
				case 0x88: Instruction_adc_reg(State.B); m_Cycles = 4; break;
				case 0x89: Instruction_adc_reg(State.C); m_Cycles = 4; break;
				case 0x8A: Instruction_adc_reg(State.D); m_Cycles = 4; break;
				case 0x8B: Instruction_adc_reg(State.E); m_Cycles = 4; break;
				case 0x8C: Instruction_adc_reg(State.H); m_Cycles = 4; break;
				case 0x8D: Instruction_adc_reg(State.L); m_Cycles = 4; break;
				case 0x8E: Instruction_adc_hl(); m_Cycles = 8; break;
				case 0x8F: Instruction_adc_reg(State.A); m_Cycles = 4; break;
				case 0xCE: Instruction_adc_reg(opcode[1]); State.PC++; m_Cycles = 8; break;

					// "SUB A reg" B:1 C:4 FLAGS: Z 1 H C
				case 0x90: Instruction_sub_reg(State.B); m_Cycles = 4; break;
				case 0x91: Instruction_sub_reg(State.C); m_Cycles = 4; break;
				case 0x92: Instruction_sub_reg(State.D); m_Cycles = 4; break;
				case 0x93: Instruction_sub_reg(State.E); m_Cycles = 4; break;
				case 0x94: Instruction_sub_reg(State.H); m_Cycles = 4; break;
				case 0x95: Instruction_sub_reg(State.L); m_Cycles = 4; break;
				case 0x96: Instruction_sub_hl(); m_Cycles = 8; break;
				case 0x97:
				{
					// SUB A A is a special case where the flags need to be 1 1 0 0
					Instruction_sub_reg(State.A);
					SetCPUFlag(FLAG_ZERO, true);
					SetCPUFlag(FLAG_SUBTRACT, true);
					SetCPUFlag(FLAG_HALF_CARRY, false);
					SetCPUFlag(FLAG_CARRY, false);
					m_Cycles = 4;
					break;
				}
				case 0xD6: Instruction_sub_reg(opcode[1]); State.PC++; m_Cycles = 8; break;

					// "SBC A reg" B:1 C:4 FLAGS: Z 1 H C
				case 0x98: Instruction_sbc_reg(State.B); m_Cycles = 4; break;
				case 0x99: Instruction_sbc_reg(State.C); m_Cycles = 4; break;
				case 0x9A: Instruction_sbc_reg(State.D); m_Cycles = 4; break;
				case 0x9B: Instruction_sbc_reg(State.E); m_Cycles = 4; break;
				case 0x9C: Instruction_sbc_reg(State.H); m_Cycles = 4; break;
				case 0x9D: Instruction_sbc_reg(State.L); m_Cycles = 4; break;
				case 0x9E: Instruction_sbc_hl(); m_Cycles = 8; break;
				case 0x9F: Instruction_sbc_reg(State.A); m_Cycles = 4; break;
				case 0xDE: Instruction_sbc_reg(opcode[1]); State.PC++; m_Cycles = 8; break;

					// "AND A reg" B:1 C:4 FLAGS: Z 0 1 0
				case 0xA0: Instruction_and_reg(State.B); m_Cycles = 4; break;
				case 0xA1: Instruction_and_reg(State.C); m_Cycles = 4; break;
				case 0xA2: Instruction_and_reg(State.D); m_Cycles = 4; break;
				case 0xA3: Instruction_and_reg(State.E); m_Cycles = 4; break;
				case 0xA4: Instruction_and_reg(State.H); m_Cycles = 4; break;
				case 0xA5: Instruction_and_reg(State.L); m_Cycles = 4; break;
				case 0xA6: Instruction_and_hl(); m_Cycles = 8; break;
				case 0xA7: Instruction_and_reg(State.A); m_Cycles = 4; break;
				case 0xE6: Instruction_and_reg(opcode[1]); State.PC++; m_Cycles = 8; break;

					// "XOR A reg" B:1 C:4 FLAGS: Z 0 0 0
				case 0xA8: Instruction_xor_reg(State.B); m_Cycles = 4; break;
				case 0xA9: Instruction_xor_reg(State.C); m_Cycles = 4; break;
				case 0xAA: Instruction_xor_reg(State.D); m_Cycles = 4; break;
				case 0xAB: Instruction_xor_reg(State.E); m_Cycles = 4; break;
				case 0xAC: Instruction_xor_reg(State.H); m_Cycles = 4; break;
				case 0xAD: Instruction_xor_reg(State.L); m_Cycles = 4; break;
				case 0xAE: Instruction_xor_hl(); m_Cycles = 8; break;
				case 0xAF: Instruction_xor_reg(State.A); m_Cycles = 4; break;
				case 0xEE: Instruction_xor_reg(opcode[1]); State.PC++; m_Cycles = 8; break;

					// "OR A reg" B:1 C:4 FLAGS: Z 0 0 0
				case 0xB0: Instruction_or_reg(State.B); m_Cycles = 4; break;
				case 0xB1: Instruction_or_reg(State.C); m_Cycles = 4; break;
				case 0xB2: Instruction_or_reg(State.D); m_Cycles = 4; break;
				case 0xB3: Instruction_or_reg(State.E); m_Cycles = 4; break;
				case 0xB4: Instruction_or_reg(State.H); m_Cycles = 4; break;
				case 0xB5: Instruction_or_reg(State.L); m_Cycles = 4; break;
				case 0xB6: Instruction_or_hl(); m_Cycles = 8; break;
				case 0xB7: Instruction_or_reg(State.A); m_Cycles = 4; break;
				case 0xF6: Instruction_or_reg(opcode[1]); State.PC++; m_Cycles = 8; break;

					// "CP A B" B:1 C:4 FLAGS: Z 1 H C
				case 0xB8: Instruction_cp_reg(State.B); m_Cycles = 4; break;
				case 0xB9: Instruction_cp_reg(State.C); m_Cycles = 4; break;
				case 0xBA: Instruction_cp_reg(State.D); m_Cycles = 4; break;
				case 0xBB: Instruction_cp_reg(State.E); m_Cycles = 4; break;
				case 0xBC: Instruction_cp_reg(State.H); m_Cycles = 4; break;
				case 0xBD: Instruction_cp_reg(State.L); m_Cycles = 4; break;
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
				case 0xFE: Instruction_cp_reg(opcode[1]); State.PC++; m_Cycles = 8; break;


					/********************************************************************************************
						16-bit Arithmetic/Logical Instructions
					*********************************************************************************************/

					// "INC reg16" B:1 C:8 FLAGS: - - - -
				case 0x03: Instruction_inc_reg16(State.BC); m_Cycles = 8; break;
				case 0x13: Instruction_inc_reg16(State.DE); m_Cycles = 8; break;
				case 0x23: Instruction_inc_reg16(State.HL); m_Cycles = 8; break;
				case 0x33: Instruction_inc_reg16(State.SP); m_Cycles = 8; break;

					// "DEC reg16" B:1 C:8 FLAGS: - - - -
				case 0x0B: Instruction_dec_reg16(State.BC); m_Cycles = 8; break;
				case 0x1B: Instruction_dec_reg16(State.DE); m_Cycles = 8; break;
				case 0x2B: Instruction_dec_reg16(State.HL); m_Cycles = 8; break;
				case 0x3B: Instruction_dec_reg16(State.SP); m_Cycles = 8; break;

					// "ADD HL reg16" B:1 C:8 FLAGS: - 0 H C
				case 0x09: Instruction_add_reg16(State.BC); m_Cycles = 8; break;
				case 0x19: Instruction_add_reg16(State.DE); m_Cycles = 8; break;
				case 0x29: Instruction_add_reg16(State.HL); m_Cycles = 8; break;
				case 0x39: Instruction_add_reg16(State.SP); m_Cycles = 8; break;

					// "ADD SP e8" B:2 C:16 FLAGS: 0 0 H C
				case 0xE8: Instruction_add_sp_e8(opcode[1]); State.PC++; m_Cycles = 16; break;

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

					m_Cycles = 4;
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

					m_Cycles = 4;
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

					m_Cycles = 4;
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

					m_Cycles = 4;
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

					m_Cycles = 4;
					break;
				}

				// "CPL" B:1 C:4 FLAGS: - 1 1 -
				case 0x2F:
				{
					State.A = ~State.A;
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
					UnimplementedInstruction(State, *opcode);
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

		if (m_MMU.ReadRegisterBit(HW_INTERRUPT_ENABLE, IE_VBLANK) &&
			m_MMU.ReadRegisterBit(HW_IF_INTERRUPT_FLAG, IF_VBLANK))
		{
			destinationAddress = DEST_ADDRESS_VBLANK;
			interruptFlag = IF_VBLANK;
			m_IsHalted = false;
		}

		if (m_MMU.ReadRegisterBit(HW_INTERRUPT_ENABLE, IE_LCD) &&
			m_MMU.ReadRegisterBit(HW_IF_INTERRUPT_FLAG, IF_LCD))
		{
			destinationAddress = DEST_ADDRESS_LCD_STAT;
			interruptFlag = IF_LCD;
			m_IsHalted = false;
		}

		if (m_MMU.ReadRegisterBit(HW_INTERRUPT_ENABLE, IE_TIMER) &&
			m_MMU.ReadRegisterBit(HW_IF_INTERRUPT_FLAG, IF_TIMER))
		{
			destinationAddress = DEST_ADDRESS_TIMER;
			interruptFlag = IF_TIMER;
			m_IsHalted = false;
		}

		if (m_MMU.ReadRegisterBit(HW_INTERRUPT_ENABLE, IE_SERIAL) &&
			m_MMU.ReadRegisterBit(HW_IF_INTERRUPT_FLAG, IF_SERIAL))
		{
			destinationAddress = DEST_ADDRESS_SERIAL;
			interruptFlag = IF_SERIAL;
			m_IsHalted = false;
		}

		if (m_MMU.ReadRegisterBit(HW_INTERRUPT_ENABLE, IE_JOYPAD) &&
			m_MMU.ReadRegisterBit(HW_IF_INTERRUPT_FLAG, IF_JOYPAD))
		{
			destinationAddress = DEST_ADDRESS_JOYPAD;
			interruptFlag = IF_JOYPAD;
			m_IsHalted = false;
		}

		if (destinationAddress != 0 && m_InterruptMasterFlag)
		{
			m_InterruptMasterFlag = false;;
			m_MMU.WriteRegisterBit(HW_IF_INTERRUPT_FLAG, interruptFlag, false);

			PushSP(State.PC);
			State.PC = destinationAddress;

			std::ostringstream stream;
			stream << "IRQ Write - Address: " << std::hex << destinationAddress << " Interrupt Type: ";
			Logger::Instance().Verbose(Domain::CPU, stream.str());
		}
	}

	void Cpu::Process16bitInstruction(uint16_t opcode, Cpu::m_CpuState& state)
	{
		switch (opcode)
		{
			// Most instuctions take 8 cycles
			m_Cycles = 8;

			// "RLC reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB00: Instruction_rlc_reg(State.B); break;
			case 0xCB01: Instruction_rlc_reg(State.C); break;
			case 0xCB02: Instruction_rlc_reg(State.D); break;
			case 0xCB03: Instruction_rlc_reg(State.E); break;
			case 0xCB04: Instruction_rlc_reg(State.H); break;
			case 0xCB05: Instruction_rlc_reg(State.L); break;
			case 0xCB06: Instruction_rlc_hl(); m_Cycles = 16; break;
			case 0xCB07: Instruction_rlc_reg(State.A); break;

				// "RRC reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB08: Instruction_rrc_reg(State.B); break;
			case 0xCB09: Instruction_rrc_reg(State.C); break;
			case 0xCB0A: Instruction_rrc_reg(State.D); break;
			case 0xCB0B: Instruction_rrc_reg(State.E); break;
			case 0xCB0C: Instruction_rrc_reg(State.H); break;
			case 0xCB0D: Instruction_rrc_reg(State.L); break;
			case 0xCB0E: Instruction_rrc_hl(); m_Cycles = 16; break;
			case 0xCB0F: Instruction_rrc_reg(State.A); break;

				// "RL reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB10: Instruction_rl_reg(State.B); break;
			case 0xCB11: Instruction_rl_reg(State.C); break;
			case 0xCB12: Instruction_rl_reg(State.D); break;
			case 0xCB13: Instruction_rl_reg(State.E); break;
			case 0xCB14: Instruction_rl_reg(State.H); break;
			case 0xCB15: Instruction_rl_reg(State.L); break;
			case 0xCB16: Instruction_rl_hl(); m_Cycles = 16; break;
			case 0xCB17: Instruction_rl_reg(State.A); break;

				// "RR reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB18: Instruction_rr_reg(State.B); break;
			case 0xCB19: Instruction_rr_reg(State.C); break;
			case 0xCB1A: Instruction_rr_reg(State.D); break;
			case 0xCB1B: Instruction_rr_reg(State.E); break;
			case 0xCB1C: Instruction_rr_reg(State.H); break;
			case 0xCB1D: Instruction_rr_reg(State.L); break;
			case 0xCB1E: Instruction_rr_hl(); m_Cycles = 16; break;
			case 0xCB1F: Instruction_rr_reg(State.A); break;

				// "SLA reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB20: Instruction_sla_reg(State.B); break;
			case 0xCB21: Instruction_sla_reg(State.C); break;
			case 0xCB22: Instruction_sla_reg(State.D); break;
			case 0xCB23: Instruction_sla_reg(State.E); break;
			case 0xCB24: Instruction_sla_reg(State.H); break;
			case 0xCB25: Instruction_sla_reg(State.L); break;
			case 0xCB26: Instruction_sla_hl(); m_Cycles = 16; break;
			case 0xCB27: Instruction_sla_reg(State.A); break;

				// "SRA reg" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB28: Instruction_sra_reg(State.B); break;
			case 0xCB29: Instruction_sra_reg(State.C); break;
			case 0xCB2A: Instruction_sra_reg(State.D); break;
			case 0xCB2B: Instruction_sra_reg(State.E); break;
			case 0xCB2C: Instruction_sra_reg(State.H); break;
			case 0xCB2D: Instruction_sra_reg(State.L); break;
			case 0xCB2E: Instruction_sra_hl(); m_Cycles = 16; break;
			case 0xCB2F: Instruction_sra_reg(State.A); break;

				// "SWAP reg" B:2 C:8 FLAGS: Z 0 0 0
			case 0xCB30: Instruction_swap_reg(State.B); break;
			case 0xCB31: Instruction_swap_reg(State.C); break;
			case 0xCB32: Instruction_swap_reg(State.D); break;
			case 0xCB33: Instruction_swap_reg(State.E); break;
			case 0xCB34: Instruction_swap_reg(State.H); break;
			case 0xCB35: Instruction_swap_reg(State.L); break;
			case 0xCB36: Instruction_swap_hl(); m_Cycles = 16; break;
			case 0xCB37: Instruction_swap_reg(State.A); break;

				// "SRL bit" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB38: Instruction_srl_reg(State.B); break;
			case 0xCB39: Instruction_srl_reg(State.C); break;
			case 0xCB3A: Instruction_srl_reg(State.D); break;
			case 0xCB3B: Instruction_srl_reg(State.E); break;
			case 0xCB3C: Instruction_srl_reg(State.H); break;
			case 0xCB3D: Instruction_srl_reg(State.L); break;
			case 0xCB3E: Instruction_srl_hl(); m_Cycles = 16; break;
			case 0xCB3F: Instruction_srl_reg(State.A); break;

				// "BIT bit, reg" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB40: Instruction_bit_bit_reg(State.B, 0); break;
			case 0xCB41: Instruction_bit_bit_reg(State.C, 0); break;
			case 0xCB42: Instruction_bit_bit_reg(State.D, 0); break;
			case 0xCB43: Instruction_bit_bit_reg(State.E, 0); break;
			case 0xCB44: Instruction_bit_bit_reg(State.H, 0); break;
			case 0xCB45: Instruction_bit_bit_reg(State.L, 0); break;
			case 0xCB46: Instruction_bit_bit_hl(0); m_Cycles = 12; break;
			case 0xCB47: Instruction_bit_bit_reg(State.A, 0); break;
			case 0xCB48: Instruction_bit_bit_reg(State.B, 1); break;
			case 0xCB49: Instruction_bit_bit_reg(State.C, 1); break;
			case 0xCB4A: Instruction_bit_bit_reg(State.D, 1); break;
			case 0xCB4B: Instruction_bit_bit_reg(State.E, 1); break;
			case 0xCB4C: Instruction_bit_bit_reg(State.H, 1); break;
			case 0xCB4D: Instruction_bit_bit_reg(State.L, 1); break;
			case 0xCB4E: Instruction_bit_bit_hl(1); m_Cycles = 12; break;
			case 0xCB4F: Instruction_bit_bit_reg(State.A, 1); break;
			case 0xCB50: Instruction_bit_bit_reg(State.B, 2); break;
			case 0xCB51: Instruction_bit_bit_reg(State.C, 2); break;
			case 0xCB52: Instruction_bit_bit_reg(State.D, 2); break;
			case 0xCB53: Instruction_bit_bit_reg(State.E, 2); break;
			case 0xCB54: Instruction_bit_bit_reg(State.H, 2); break;
			case 0xCB55: Instruction_bit_bit_reg(State.L, 2); break;
			case 0xCB56: Instruction_bit_bit_hl(2); m_Cycles = 12; break;
			case 0xCB57: Instruction_bit_bit_reg(State.A, 2); break;
			case 0xCB58: Instruction_bit_bit_reg(State.B, 3); break;
			case 0xCB59: Instruction_bit_bit_reg(State.C, 3); break;
			case 0xCB5A: Instruction_bit_bit_reg(State.D, 3); break;
			case 0xCB5B: Instruction_bit_bit_reg(State.E, 3); break;
			case 0xCB5C: Instruction_bit_bit_reg(State.H, 3); break;
			case 0xCB5D: Instruction_bit_bit_reg(State.L, 3); break;
			case 0xCB5E: Instruction_bit_bit_hl(3); m_Cycles = 12; break;
			case 0xCB5F: Instruction_bit_bit_reg(State.A, 3); break;
			case 0xCB60: Instruction_bit_bit_reg(State.B, 4); break;
			case 0xCB61: Instruction_bit_bit_reg(State.C, 4); break;
			case 0xCB62: Instruction_bit_bit_reg(State.D, 4); break;
			case 0xCB63: Instruction_bit_bit_reg(State.E, 4); break;
			case 0xCB64: Instruction_bit_bit_reg(State.H, 4); break;
			case 0xCB65: Instruction_bit_bit_reg(State.L, 4); break;
			case 0xCB66: Instruction_bit_bit_hl(4); m_Cycles = 12; break;
			case 0xCB67: Instruction_bit_bit_reg(State.A, 4); break;
			case 0xCB68: Instruction_bit_bit_reg(State.B, 5); break;
			case 0xCB69: Instruction_bit_bit_reg(State.C, 5); break;
			case 0xCB6A: Instruction_bit_bit_reg(State.D, 5); break;
			case 0xCB6B: Instruction_bit_bit_reg(State.E, 5); break;
			case 0xCB6C: Instruction_bit_bit_reg(State.H, 5); break;
			case 0xCB6D: Instruction_bit_bit_reg(State.L, 5); break;
			case 0xCB6E: Instruction_bit_bit_hl(5); m_Cycles = 12; break;
			case 0xCB6F: Instruction_bit_bit_reg(State.A, 5); break;
			case 0xCB70: Instruction_bit_bit_reg(State.B, 6); break;
			case 0xCB71: Instruction_bit_bit_reg(State.C, 6); break;
			case 0xCB72: Instruction_bit_bit_reg(State.D, 6); break;
			case 0xCB73: Instruction_bit_bit_reg(State.E, 6); break;
			case 0xCB74: Instruction_bit_bit_reg(State.H, 6); break;
			case 0xCB75: Instruction_bit_bit_reg(State.L, 6); break;
			case 0xCB76: Instruction_bit_bit_hl(6); m_Cycles = 12; break;
			case 0xCB77: Instruction_bit_bit_reg(State.A, 6); break;
			case 0xCB78: Instruction_bit_bit_reg(State.B, 7); break;
			case 0xCB79: Instruction_bit_bit_reg(State.C, 7); break;
			case 0xCB7A: Instruction_bit_bit_reg(State.D, 7); break;
			case 0xCB7B: Instruction_bit_bit_reg(State.E, 7); break;
			case 0xCB7C: Instruction_bit_bit_reg(State.H, 7); break;
			case 0xCB7D: Instruction_bit_bit_reg(State.L, 7); break;
			case 0xCB7E: Instruction_bit_bit_hl(7); m_Cycles = 12; break;
			case 0xCB7F: Instruction_bit_bit_reg(State.A, 7); break;

				// "RES bit, reg" B:2 C:8 FLAGS: - - - -
			case 0xCB80: Instruction_res_bit_reg(State.B, 0); break;
			case 0xCB81: Instruction_res_bit_reg(State.C, 0); break;
			case 0xCB82: Instruction_res_bit_reg(State.D, 0); break;
			case 0xCB83: Instruction_res_bit_reg(State.E, 0); break;
			case 0xCB84: Instruction_res_bit_reg(State.H, 0); break;
			case 0xCB85: Instruction_res_bit_reg(State.L, 0); break;
			case 0xCB86: Instruction_res_bit_hl(0); m_Cycles = 16; break;
			case 0xCB87: Instruction_res_bit_reg(State.A, 0); break;
			case 0xCB88: Instruction_res_bit_reg(State.B, 1); break;
			case 0xCB89: Instruction_res_bit_reg(State.C, 1); break;
			case 0xCB8A: Instruction_res_bit_reg(State.D, 1); break;
			case 0xCB8B: Instruction_res_bit_reg(State.E, 1); break;
			case 0xCB8C: Instruction_res_bit_reg(State.H, 1); break;
			case 0xCB8D: Instruction_res_bit_reg(State.L, 1); break;
			case 0xCB8E: Instruction_res_bit_hl(1); m_Cycles = 16; break;
			case 0xCB8F: Instruction_res_bit_reg(State.A, 1); break;
			case 0xCB90: Instruction_res_bit_reg(State.B, 2); break;
			case 0xCB91: Instruction_res_bit_reg(State.C, 2); break;
			case 0xCB92: Instruction_res_bit_reg(State.D, 2); break;
			case 0xCB93: Instruction_res_bit_reg(State.E, 2); break;
			case 0xCB94: Instruction_res_bit_reg(State.H, 2); break;
			case 0xCB95: Instruction_res_bit_reg(State.L, 2); break;
			case 0xCB96: Instruction_res_bit_hl(2); m_Cycles = 16; break;
			case 0xCB97: Instruction_res_bit_reg(State.A, 2); break;
			case 0xCB98: Instruction_res_bit_reg(State.B, 3); break;
			case 0xCB99: Instruction_res_bit_reg(State.C, 3); break;
			case 0xCB9A: Instruction_res_bit_reg(State.D, 3); break;
			case 0xCB9B: Instruction_res_bit_reg(State.E, 3); break;
			case 0xCB9C: Instruction_res_bit_reg(State.H, 3); break;
			case 0xCB9D: Instruction_res_bit_reg(State.L, 3); break;
			case 0xCB9E: Instruction_res_bit_hl(3); m_Cycles = 16; break;
			case 0xCB9F: Instruction_res_bit_reg(State.A, 3); break;
			case 0xCBA0: Instruction_res_bit_reg(State.B, 4); break;
			case 0xCBA1: Instruction_res_bit_reg(State.C, 4); break;
			case 0xCBA2: Instruction_res_bit_reg(State.D, 4); break;
			case 0xCBA3: Instruction_res_bit_reg(State.E, 4); break;
			case 0xCBA4: Instruction_res_bit_reg(State.H, 4); break;
			case 0xCBA5: Instruction_res_bit_reg(State.L, 4); break;
			case 0xCBA6: Instruction_res_bit_hl(4); m_Cycles = 16; break;
			case 0xCBA7: Instruction_res_bit_reg(State.A, 4); break;
			case 0xCBA8: Instruction_res_bit_reg(State.B, 5); break;
			case 0xCBA9: Instruction_res_bit_reg(State.C, 5); break;
			case 0xCBAA: Instruction_res_bit_reg(State.D, 5); break;
			case 0xCBAB: Instruction_res_bit_reg(State.E, 5); break;
			case 0xCBAC: Instruction_res_bit_reg(State.H, 5); break;
			case 0xCBAD: Instruction_res_bit_reg(State.L, 5); break;
			case 0xCBAE: Instruction_res_bit_hl(5); m_Cycles = 16; break;
			case 0xCBAF: Instruction_res_bit_reg(State.A, 5); break;
			case 0xCBB0: Instruction_res_bit_reg(State.B, 6); break;
			case 0xCBB1: Instruction_res_bit_reg(State.C, 6); break;
			case 0xCBB2: Instruction_res_bit_reg(State.D, 6); break;
			case 0xCBB3: Instruction_res_bit_reg(State.E, 6); break;
			case 0xCBB4: Instruction_res_bit_reg(State.H, 6); break;
			case 0xCBB5: Instruction_res_bit_reg(State.L, 6); break;
			case 0xCBB6: Instruction_res_bit_hl(6); m_Cycles = 16; break;
			case 0xCBB7: Instruction_res_bit_reg(State.A, 6); break;
			case 0xCBB8: Instruction_res_bit_reg(State.B, 7); break;
			case 0xCBB9: Instruction_res_bit_reg(State.C, 7); break;
			case 0xCBBA: Instruction_res_bit_reg(State.D, 7); break;
			case 0xCBBB: Instruction_res_bit_reg(State.E, 7); break;
			case 0xCBBC: Instruction_res_bit_reg(State.H, 7); break;
			case 0xCBBD: Instruction_res_bit_reg(State.L, 7); break;
			case 0xCBBE: Instruction_res_bit_hl(7); m_Cycles = 16; break;
			case 0xCBBF: Instruction_res_bit_reg(State.A, 7); break;

				// "SET bit, register" B:2 C:8 - - - -
			case 0xCBC0: Instruction_set_bit_reg(State.B, 0); break;
			case 0xCBC1: Instruction_set_bit_reg(State.C, 0); break;
			case 0xCBC2: Instruction_set_bit_reg(State.D, 0); break;
			case 0xCBC3: Instruction_set_bit_reg(State.E, 0); break;
			case 0xCBC4: Instruction_set_bit_reg(State.H, 0); break;
			case 0xCBC5: Instruction_set_bit_reg(State.L, 0); break;
			case 0xCBC6: Instruction_set_bit_hl(0); m_Cycles = 16; break;
			case 0xCBC7: Instruction_set_bit_reg(State.A, 0); break;
			case 0xCBC8: Instruction_set_bit_reg(State.B, 1); break;
			case 0xCBC9: Instruction_set_bit_reg(State.C, 1); break;
			case 0xCBCA: Instruction_set_bit_reg(State.D, 1); break;
			case 0xCBCB: Instruction_set_bit_reg(State.E, 1); break;
			case 0xCBCC: Instruction_set_bit_reg(State.H, 1); break;
			case 0xCBCD: Instruction_set_bit_reg(State.L, 1); break;
			case 0xCBCE: Instruction_set_bit_hl(1); m_Cycles = 16; break;
			case 0xCBCF: Instruction_set_bit_reg(State.A, 1); break;
			case 0xCBD0: Instruction_set_bit_reg(State.B, 2); break;
			case 0xCBD1: Instruction_set_bit_reg(State.C, 2); break;
			case 0xCBD2: Instruction_set_bit_reg(State.D, 2); break;
			case 0xCBD3: Instruction_set_bit_reg(State.E, 2); break;
			case 0xCBD4: Instruction_set_bit_reg(State.H, 2); break;
			case 0xCBD5: Instruction_set_bit_reg(State.L, 2); break;
			case 0xCBD6: Instruction_set_bit_hl(2); m_Cycles = 16; break;
			case 0xCBD7: Instruction_set_bit_reg(State.A, 2); break;
			case 0xCBD8: Instruction_set_bit_reg(State.B, 3); break;
			case 0xCBD9: Instruction_set_bit_reg(State.C, 3); break;
			case 0xCBDA: Instruction_set_bit_reg(State.D, 3); break;
			case 0xCBDB: Instruction_set_bit_reg(State.E, 3); break;
			case 0xCBDC: Instruction_set_bit_reg(State.H, 3); break;
			case 0xCBDD: Instruction_set_bit_reg(State.L, 3); break;
			case 0xCBDE: Instruction_set_bit_hl(3); m_Cycles = 16; break;
			case 0xCBDF: Instruction_set_bit_reg(State.A, 3); break;
			case 0xCBE0: Instruction_set_bit_reg(State.B, 4); break;
			case 0xCBE1: Instruction_set_bit_reg(State.C, 4); break;
			case 0xCBE2: Instruction_set_bit_reg(State.D, 4); break;
			case 0xCBE3: Instruction_set_bit_reg(State.E, 4); break;
			case 0xCBE4: Instruction_set_bit_reg(State.H, 4); break;
			case 0xCBE5: Instruction_set_bit_reg(State.L, 4); break;
			case 0xCBE6: Instruction_set_bit_hl(4); m_Cycles = 16; break;
			case 0xCBE7: Instruction_set_bit_reg(State.A, 4); break;
			case 0xCBE8: Instruction_set_bit_reg(State.B, 5); break;
			case 0xCBE9: Instruction_set_bit_reg(State.C, 5); break;
			case 0xCBEA: Instruction_set_bit_reg(State.D, 5); break;
			case 0xCBEB: Instruction_set_bit_reg(State.E, 5); break;
			case 0xCBEC: Instruction_set_bit_reg(State.H, 5); break;
			case 0xCBED: Instruction_set_bit_reg(State.L, 5); break;
			case 0xCBEE: Instruction_set_bit_hl(5); m_Cycles = 16; break;
			case 0xCBEF: Instruction_set_bit_reg(State.A, 5); break;
			case 0xCBF0: Instruction_set_bit_reg(State.B, 6); break;
			case 0xCBF1: Instruction_set_bit_reg(State.C, 6); break;
			case 0xCBF2: Instruction_set_bit_reg(State.D, 6); break;
			case 0xCBF3: Instruction_set_bit_reg(State.E, 6); break;
			case 0xCBF4: Instruction_set_bit_reg(State.H, 6); break;
			case 0xCBF5: Instruction_set_bit_reg(State.L, 6); break;
			case 0xCBF6: Instruction_set_bit_hl(6); m_Cycles = 16; break;
			case 0xCBF7: Instruction_set_bit_reg(State.A, 6); break;
			case 0xCBF8: Instruction_set_bit_reg(State.B, 7); break;
			case 0xCBF9: Instruction_set_bit_reg(State.C, 7); break;
			case 0xCBFA: Instruction_set_bit_reg(State.D, 7); break;
			case 0xCBFB: Instruction_set_bit_reg(State.E, 7); break;
			case 0xCBFC: Instruction_set_bit_reg(State.H, 7); break;
			case 0xCBFD: Instruction_set_bit_reg(State.L, 7); break;
			case 0xCBFE: Instruction_set_bit_hl(7); m_Cycles = 16; break;
			case 0xCBFF: Instruction_set_bit_reg(State.A, 7); break;

			default:
				UnimplementedInstruction(State, static_cast<uint8_t>(opcode));
				break;
		}
	}

	void Cpu::UnimplementedInstruction(Cpu::m_CpuState& State, uint8_t opcode)
	{
		Logger::Instance().Error(Domain::CPU, "Error: Unimplemented instruction: %02x"/*, opcode */);
	}

	int Cpu::Disassemble(uint8_t* opcode, int pc)
	{
		int opBytes = 1;

		switch (*opcode)
		{
			case 0x00: OutputDisassembledInstruction("NOP", pc, opcode, 1); break; // "NOP" B:1 C:4 FLAGS: - - - -
			case 0x01: OutputDisassembledInstruction("LD BC n16", pc, opcode, 3); opBytes = 3; break; // "LD BC n16" B:3 C:12 FLAGS: - - - -
			case 0x02: OutputDisassembledInstruction("LD [BC] A", pc, opcode, 1); break; // "LD [BC] A" B:1 C:8 FLAGS: - - - -
			case 0x03: OutputDisassembledInstruction("INC BC", pc, opcode, 1); break; // "INC BC" B:1 C:8 FLAGS: - - - -
			case 0x04: OutputDisassembledInstruction("INC B", pc, opcode, 1); break; // "INC B" B:1 C:4 FLAGS: Z 0 H -
			case 0x05: OutputDisassembledInstruction("DEC B", pc, opcode, 1); break; // "DEC B" B:1 C:4 FLAGS: Z 1 H -
			case 0x06: OutputDisassembledInstruction("LD B n8", pc, opcode, 2); opBytes = 2; break; // "LD B n8" B:2 C:8 FLAGS: - - - -
			case 0x07: OutputDisassembledInstruction("RLCA", pc, opcode, 1); break; // "RLCA" B:1 C:4 FLAGS: 0 0 0 C
			case 0x08: OutputDisassembledInstruction("LD [a16] SP", pc, opcode, 3); opBytes = 3; break; // "LD [a16] SP" B:3 C:20 FLAGS: - - - -
			case 0x09: OutputDisassembledInstruction("ADD HL BC", pc, opcode, 1); break; // "ADD HL BC" B:1 C:8 FLAGS: - 0 H C
			case 0x0A: OutputDisassembledInstruction("LD A [BC]", pc, opcode, 1); break; // "LD A [BC]" B:1 C:8 FLAGS: - - - -
			case 0x0B: OutputDisassembledInstruction("DEC BC", pc, opcode, 1); break; // "DEC BC" B:1 C:8 FLAGS: - - - -
			case 0x0C: OutputDisassembledInstruction("INC C", pc, opcode, 1); break; // "INC C" B:1 C:4 FLAGS: Z 0 H -
			case 0x0D: OutputDisassembledInstruction("DEC C", pc, opcode, 1); break; // "DEC C" B:1 C:4 FLAGS: Z 1 H -
			case 0x0E: OutputDisassembledInstruction("LD C n8", pc, opcode, 2); opBytes = 2; break; // "LD C n8" B:2 C:8 FLAGS: - - - -
			case 0x0F: OutputDisassembledInstruction("RRCA", pc, opcode, 1); break; // "RRCA" B:1 C:4 FLAGS: 0 0 0 C

			case 0x10: OutputDisassembledInstruction("STOP n8", pc, opcode, 2); opBytes = 2; break; // "STOP n8" B:2 C:4 FLAGS: - - - -
			case 0x11: OutputDisassembledInstruction("LD DE n16", pc, opcode, 3); opBytes = 3; break; // "LD DE n16" B:3 C:12 FLAGS: - - - -
			case 0x12: OutputDisassembledInstruction("LD [DE] A", pc, opcode, 1); break; // "LD [DE] A" B:1 C:8 FLAGS: - - - -
			case 0x13: OutputDisassembledInstruction("INC DE", pc, opcode, 1); break; // "INC DE" B:1 C:8 FLAGS: - - - -
			case 0x14: OutputDisassembledInstruction("INC D", pc, opcode, 1); break; // "INC D" B:1 C:4 FLAGS: Z 0 H -
			case 0x15: OutputDisassembledInstruction("DEC D", pc, opcode, 1); break; // "DEC D" B:1 C:4 FLAGS: Z 1 H -
			case 0x16: OutputDisassembledInstruction("LD D n8", pc, opcode, 2); opBytes = 2; break; // "LD D n8" B:2 C:8 FLAGS: - - - -
			case 0x17: OutputDisassembledInstruction("RLA", pc, opcode, 1); break; // "RLA" B:1 C:4 FLAGS: 0 0 0 C
			case 0x18: OutputDisassembledInstruction("JR e8", pc, opcode, 2); opBytes = 2; break; // "JR e8" B:2 C:12 FLAGS: - - - -
			case 0x19: OutputDisassembledInstruction("ADD HL DE", pc, opcode, 1); break; // "ADD HL DE" B:1 C:8 FLAGS: - 0 H C
			case 0x1A: OutputDisassembledInstruction("LD A [DE]", pc, opcode, 1); break; // "LD A [DE]" B:1 C:8 FLAGS: - - - -
			case 0x1B: OutputDisassembledInstruction("DEC DE", pc, opcode, 1); break; // "DEC DE" B:1 C:8 FLAGS: - - - -
			case 0x1C: OutputDisassembledInstruction("INC E", pc, opcode, 1); break; // "INC E" B:1 C:4 FLAGS: Z 0 H -
			case 0x1D: OutputDisassembledInstruction("DEC E", pc, opcode, 1); break; // "DEC E" B:1 C:4 FLAGS: Z 1 H -
			case 0x1E: OutputDisassembledInstruction("LD E n8", pc, opcode, 2); opBytes = 2; break; // "LD E n8" B:2 C:8 FLAGS: - - - -
			case 0x1F: OutputDisassembledInstruction("RRA", pc, opcode, 1); break; // "RRA" B:1 C:4 FLAGS: 0 0 0 C

			case 0x20: OutputDisassembledInstruction("JR NZ e8", pc, opcode, 2); opBytes = 2; break; // "JR NZ e8" B:2 C:128 FLAGS: - - - -
			case 0x21: OutputDisassembledInstruction("LD HL n16", pc, opcode, 3); opBytes = 3; break; // "LD HL n16" B:3 C:12 FLAGS: - - - -
			case 0x22: OutputDisassembledInstruction("LD [HL] A", pc, opcode, 1); break; // "LD [HL] A" B:1 C:8 FLAGS: - - - -
			case 0x23: OutputDisassembledInstruction("INC HL", pc, opcode, 1); break; // "INC HL" B:1 C:8 FLAGS: - - - -
			case 0x24: OutputDisassembledInstruction("INC H", pc, opcode, 1); break; // "INC H" B:1 C:4 FLAGS: Z 0 H -
			case 0x25: OutputDisassembledInstruction("DEC H", pc, opcode, 1); break; // "DEC H" B:1 C:4 FLAGS: Z 1 H -
			case 0x26: OutputDisassembledInstruction("LD H n8", pc, opcode, 2); opBytes = 2; break; // "LD H n8" B:2 C:8 FLAGS: - - - -
			case 0x27: OutputDisassembledInstruction("DAA", pc, opcode, 1); break; // "DAA" B:1 C:4 FLAGS: Z - 0 C
			case 0x28: OutputDisassembledInstruction("JR Z e8", pc, opcode, 2); opBytes = 2; break; // "JR Z e8" B:2 C:128 FLAGS: - - - -
			case 0x29: OutputDisassembledInstruction("ADD HL HL", pc, opcode, 1); break; // "ADD HL HL" B:1 C:8 FLAGS: - 0 H C
			case 0x2A: OutputDisassembledInstruction("LD A [HL]", pc, opcode, 1); break; // "LD A [HL]" B:1 C:8 FLAGS: - - - -
			case 0x2B: OutputDisassembledInstruction("DEC HL", pc, opcode, 1); break; // "DEC HL" B:1 C:8 FLAGS: - - - -
			case 0x2C: OutputDisassembledInstruction("INC L", pc, opcode, 1); break; // "INC L" B:1 C:4 FLAGS: Z 0 H -
			case 0x2D: OutputDisassembledInstruction("DEC L", pc, opcode, 1); break; // "DEC L" B:1 C:4 FLAGS: Z 1 H -
			case 0x2E: OutputDisassembledInstruction("LD L n8", pc, opcode, 2); opBytes = 2; break; // "LD L n8" B:2 C:8 FLAGS: - - - -
			case 0x2F: OutputDisassembledInstruction("CPL", pc, opcode, 1); break; // "CPL" B:1 C:4 FLAGS: - 1 1 -

			case 0x30: OutputDisassembledInstruction("JR NC e8", pc, opcode, 2); opBytes = 2; break; // "JR NC e8" B:2 C:128 FLAGS: - - - -
			case 0x31: OutputDisassembledInstruction("LD SP n16", pc, opcode, 3); opBytes = 3; break; // "LD SP n16" B:3 C:12 FLAGS: - - - -
			case 0x32: OutputDisassembledInstruction("LD [HL] A", pc, opcode, 1); break; // "LD [HL] A" B:1 C:8 FLAGS: - - - -
			case 0x33: OutputDisassembledInstruction("INC SP", pc, opcode, 1); break; // "INC SP" B:1 C:8 FLAGS: - - - -
			case 0x34: OutputDisassembledInstruction("INC [HL]", pc, opcode, 1); break; // "INC [HL]" B:1 C:12 FLAGS: Z 0 H -
			case 0x35: OutputDisassembledInstruction("DEC [HL]", pc, opcode, 1); break; // "DEC [HL]" B:1 C:12 FLAGS: Z 1 H -
			case 0x36: OutputDisassembledInstruction("LD [HL] n8", pc, opcode, 2); opBytes = 2; break; // "LD [HL] n8" B:2 C:12 FLAGS: - - - -
			case 0x37: OutputDisassembledInstruction("SCF", pc, opcode, 1); break; // "SCF" B:1 C:4 FLAGS: - 0 0 1
			case 0x38: OutputDisassembledInstruction("JR C e8", pc, opcode, 2); opBytes = 2; break; // "JR C e8" B:2 C:128 FLAGS: - - - -
			case 0x39: OutputDisassembledInstruction("ADD HL SP", pc, opcode, 1); break; // "ADD HL SP" B:1 C:8 FLAGS: - 0 H C
			case 0x3A: OutputDisassembledInstruction("LD A [HL]", pc, opcode, 1); break; // "LD A [HL]" B:1 C:8 FLAGS: - - - -
			case 0x3B: OutputDisassembledInstruction("DEC SP", pc, opcode, 1); break; // "DEC SP" B:1 C:8 FLAGS: - - - -
			case 0x3C: OutputDisassembledInstruction("INC A", pc, opcode, 1); break; // "INC A" B:1 C:4 FLAGS: Z 0 H -
			case 0x3D: OutputDisassembledInstruction("DEC A", pc, opcode, 1); break; // "DEC A" B:1 C:4 FLAGS: Z 1 H -
			case 0x3E: OutputDisassembledInstruction("LD A n8", pc, opcode, 2); opBytes = 2; break; // "LD A n8" B:2 C:8 FLAGS: - - - -
			case 0x3F: OutputDisassembledInstruction("CCF", pc, opcode, 1); break; // "CCF" B:1 C:4 FLAGS: - 0 0 C

			case 0x40: OutputDisassembledInstruction("LD B B", pc, opcode, 1); break; // "LD B B" B:1 C:4 FLAGS: - - - -
			case 0x41: OutputDisassembledInstruction("LD B C", pc, opcode, 1); break; // "LD B C" B:1 C:4 FLAGS: - - - -
			case 0x42: OutputDisassembledInstruction("LD B D", pc, opcode, 1); break; // "LD B D" B:1 C:4 FLAGS: - - - -
			case 0x43: OutputDisassembledInstruction("LD B E", pc, opcode, 1); break; // "LD B E" B:1 C:4 FLAGS: - - - -
			case 0x44: OutputDisassembledInstruction("LD B H", pc, opcode, 1); break; // "LD B H" B:1 C:4 FLAGS: - - - -
			case 0x45: OutputDisassembledInstruction("LD B L", pc, opcode, 1); break; // "LD B L" B:1 C:4 FLAGS: - - - -
			case 0x46: OutputDisassembledInstruction("LD B [HL]", pc, opcode, 1); break; // "LD B [HL]" B:1 C:8 FLAGS: - - - -
			case 0x47: OutputDisassembledInstruction("LD B A", pc, opcode, 1); break; // "LD B A" B:1 C:4 FLAGS: - - - -
			case 0x48: OutputDisassembledInstruction("LD C B", pc, opcode, 1); break; // "LD C B" B:1 C:4 FLAGS: - - - -
			case 0x49: OutputDisassembledInstruction("LD C C", pc, opcode, 1); break; // "LD C C" B:1 C:4 FLAGS: - - - -
			case 0x4A: OutputDisassembledInstruction("LD C D", pc, opcode, 1); break; // "LD C D" B:1 C:4 FLAGS: - - - -
			case 0x4B: OutputDisassembledInstruction("LD C E", pc, opcode, 1); break; // "LD C E" B:1 C:4 FLAGS: - - - -
			case 0x4C: OutputDisassembledInstruction("LD C H", pc, opcode, 1); break; // "LD C H" B:1 C:4 FLAGS: - - - -
			case 0x4D: OutputDisassembledInstruction("LD C L", pc, opcode, 1); break; // "LD C L" B:1 C:4 FLAGS: - - - -
			case 0x4E: OutputDisassembledInstruction("LD C [HL]", pc, opcode, 1); break; // "LD C [HL]" B:1 C:8 FLAGS: - - - -
			case 0x4F: OutputDisassembledInstruction("LD C A", pc, opcode, 1); break; // "LD C A" B:1 C:4 FLAGS: - - - -

			case 0x50: OutputDisassembledInstruction("LD D B", pc, opcode, 1); break; // "LD D B" B:1 C:4 FLAGS: - - - -
			case 0x51: OutputDisassembledInstruction("LD D C", pc, opcode, 1); break; // "LD D C" B:1 C:4 FLAGS: - - - -
			case 0x52: OutputDisassembledInstruction("LD D D", pc, opcode, 1); break; // "LD D D" B:1 C:4 FLAGS: - - - -
			case 0x53: OutputDisassembledInstruction("LD D E", pc, opcode, 1); break; // "LD D E" B:1 C:4 FLAGS: - - - -
			case 0x54: OutputDisassembledInstruction("LD D H", pc, opcode, 1); break; // "LD D H" B:1 C:4 FLAGS: - - - -
			case 0x55: OutputDisassembledInstruction("LD D L", pc, opcode, 1); break; // "LD D L" B:1 C:4 FLAGS: - - - -
			case 0x56: OutputDisassembledInstruction("LD D [HL]", pc, opcode, 1); break; // "LD D [HL]" B:1 C:8 FLAGS: - - - -
			case 0x57: OutputDisassembledInstruction("LD D A", pc, opcode, 1); break; // "LD D A" B:1 C:4 FLAGS: - - - -
			case 0x58: OutputDisassembledInstruction("LD E B", pc, opcode, 1); break; // "LD E B" B:1 C:4 FLAGS: - - - -
			case 0x59: OutputDisassembledInstruction("LD E C", pc, opcode, 1); break; // "LD E C" B:1 C:4 FLAGS: - - - -
			case 0x5A: OutputDisassembledInstruction("LD E D", pc, opcode, 1); break; // "LD E D" B:1 C:4 FLAGS: - - - -
			case 0x5B: OutputDisassembledInstruction("LD E E", pc, opcode, 1); break; // "LD E E" B:1 C:4 FLAGS: - - - -
			case 0x5C: OutputDisassembledInstruction("LD E H", pc, opcode, 1); break; // "LD E H" B:1 C:4 FLAGS: - - - -
			case 0x5D: OutputDisassembledInstruction("LD E L", pc, opcode, 1); break; // "LD E L" B:1 C:4 FLAGS: - - - -
			case 0x5E: OutputDisassembledInstruction("LD E [HL]", pc, opcode, 1); break; // "LD E [HL]" B:1 C:8 FLAGS: - - - -
			case 0x5F: OutputDisassembledInstruction("LD E A", pc, opcode, 1); break; // "LD E A" B:1 C:4 FLAGS: - - - -

			case 0x60: OutputDisassembledInstruction("LD H B", pc, opcode, 1); break; // "LD H B" B:1 C:4 FLAGS: - - - -
			case 0x61: OutputDisassembledInstruction("LD H C", pc, opcode, 1); break; // "LD H C" B:1 C:4 FLAGS: - - - -
			case 0x62: OutputDisassembledInstruction("LD H D", pc, opcode, 1); break; // "LD H D" B:1 C:4 FLAGS: - - - -
			case 0x63: OutputDisassembledInstruction("LD H E", pc, opcode, 1); break; // "LD H E" B:1 C:4 FLAGS: - - - -
			case 0x64: OutputDisassembledInstruction("LD H H", pc, opcode, 1); break; // "LD H H" B:1 C:4 FLAGS: - - - -
			case 0x65: OutputDisassembledInstruction("LD H L", pc, opcode, 1); break; // "LD H L" B:1 C:4 FLAGS: - - - -
			case 0x66: OutputDisassembledInstruction("LD H [HL]", pc, opcode, 1); break; // "LD H [HL]" B:1 C:8 FLAGS: - - - -
			case 0x67: OutputDisassembledInstruction("LD H A", pc, opcode, 1); break; // "LD H A" B:1 C:4 FLAGS: - - - -
			case 0x68: OutputDisassembledInstruction("LD L B", pc, opcode, 1); break; // "LD L B" B:1 C:4 FLAGS: - - - -
			case 0x69: OutputDisassembledInstruction("LD L C", pc, opcode, 1); break; // "LD L C" B:1 C:4 FLAGS: - - - -
			case 0x6A: OutputDisassembledInstruction("LD L D", pc, opcode, 1); break; // "LD L D" B:1 C:4 FLAGS: - - - -
			case 0x6B: OutputDisassembledInstruction("LD L E", pc, opcode, 1); break; // "LD L E" B:1 C:4 FLAGS: - - - -
			case 0x6C: OutputDisassembledInstruction("LD L H", pc, opcode, 1); break; // "LD L H" B:1 C:4 FLAGS: - - - -
			case 0x6D: OutputDisassembledInstruction("LD L L", pc, opcode, 1); break; // "LD L L" B:1 C:4 FLAGS: - - - -
			case 0x6E: OutputDisassembledInstruction("LD L [HL]", pc, opcode, 1); break; // "LD L [HL]" B:1 C:8 FLAGS: - - - -
			case 0x6F: OutputDisassembledInstruction("LD L A", pc, opcode, 1); break; // "LD L A" B:1 C:4 FLAGS: - - - -

			case 0x70: OutputDisassembledInstruction("LD [HL] B", pc, opcode, 1); break; // "LD [HL] B" B:1 C:8 FLAGS: - - - -
			case 0x71: OutputDisassembledInstruction("LD [HL] C", pc, opcode, 1); break; // "LD [HL] C" B:1 C:8 FLAGS: - - - -
			case 0x72: OutputDisassembledInstruction("LD [HL] D", pc, opcode, 1); break; // "LD [HL] D" B:1 C:8 FLAGS: - - - -
			case 0x73: OutputDisassembledInstruction("LD [HL] E", pc, opcode, 1); break; // "LD [HL] E" B:1 C:8 FLAGS: - - - -
			case 0x74: OutputDisassembledInstruction("LD [HL] H", pc, opcode, 1); break; // "LD [HL] H" B:1 C:8 FLAGS: - - - -
			case 0x75: OutputDisassembledInstruction("LD [HL] L", pc, opcode, 1); break; // "LD [HL] L" B:1 C:8 FLAGS: - - - -
			case 0x76: OutputDisassembledInstruction("HALT", pc, opcode, 1); break; // "HALT" B:1 C:4 FLAGS: - - - -
			case 0x77: OutputDisassembledInstruction("LD [HL] A", pc, opcode, 1); break; // "LD [HL] A" B:1 C:8 FLAGS: - - - -
			case 0x78: OutputDisassembledInstruction("LD A B", pc, opcode, 1); break; // "LD A B" B:1 C:4 FLAGS: - - - -
			case 0x79: OutputDisassembledInstruction("LD A C", pc, opcode, 1); break; // "LD A C" B:1 C:4 FLAGS: - - - -
			case 0x7A: OutputDisassembledInstruction("LD A D", pc, opcode, 1); break; // "LD A D" B:1 C:4 FLAGS: - - - -
			case 0x7B: OutputDisassembledInstruction("LD A E", pc, opcode, 1); break; // "LD A E" B:1 C:4 FLAGS: - - - -
			case 0x7C: OutputDisassembledInstruction("LD A H", pc, opcode, 1); break; // "LD A H" B:1 C:4 FLAGS: - - - -
			case 0x7D: OutputDisassembledInstruction("LD A L", pc, opcode, 1); break; // "LD A L" B:1 C:4 FLAGS: - - - -
			case 0x7E: OutputDisassembledInstruction("LD A [HL]", pc, opcode, 1); break; // "LD A [HL]" B:1 C:8 FLAGS: - - - -
			case 0x7F: OutputDisassembledInstruction("LD A A", pc, opcode, 1); break; // "LD A A" B:1 C:4 FLAGS: - - - -

			case 0x80: OutputDisassembledInstruction("ADD A B", pc, opcode, 1); break; // "ADD A B" B:1 C:4 FLAGS: Z 0 H C
			case 0x81: OutputDisassembledInstruction("ADD A C", pc, opcode, 1); break; // "ADD A C" B:1 C:4 FLAGS: Z 0 H C
			case 0x82: OutputDisassembledInstruction("ADD A D", pc, opcode, 1); break; // "ADD A D" B:1 C:4 FLAGS: Z 0 H C
			case 0x83: OutputDisassembledInstruction("ADD A E", pc, opcode, 1); break; // "ADD A E" B:1 C:4 FLAGS: Z 0 H C
			case 0x84: OutputDisassembledInstruction("ADD A H", pc, opcode, 1); break; // "ADD A H" B:1 C:4 FLAGS: Z 0 H C
			case 0x85: OutputDisassembledInstruction("ADD A L", pc, opcode, 1); break; // "ADD A L" B:1 C:4 FLAGS: Z 0 H C
			case 0x86: OutputDisassembledInstruction("ADD A [HL]", pc, opcode, 1); break; // "ADD A [HL]" B:1 C:8 FLAGS: Z 0 H C
			case 0x87: OutputDisassembledInstruction("ADD A A", pc, opcode, 1); break; // "ADD A A" B:1 C:4 FLAGS: Z 0 H C
			case 0x88: OutputDisassembledInstruction("ADC A B", pc, opcode, 1); break; // "ADC A B" B:1 C:4 FLAGS: Z 0 H C
			case 0x89: OutputDisassembledInstruction("ADC A C", pc, opcode, 1); break; // "ADC A C" B:1 C:4 FLAGS: Z 0 H C
			case 0x8A: OutputDisassembledInstruction("ADC A D", pc, opcode, 1); break; // "ADC A D" B:1 C:4 FLAGS: Z 0 H C
			case 0x8B: OutputDisassembledInstruction("ADC A E", pc, opcode, 1); break; // "ADC A E" B:1 C:4 FLAGS: Z 0 H C
			case 0x8C: OutputDisassembledInstruction("ADC A H", pc, opcode, 1); break; // "ADC A H" B:1 C:4 FLAGS: Z 0 H C
			case 0x8D: OutputDisassembledInstruction("ADC A L", pc, opcode, 1); break; // "ADC A L" B:1 C:4 FLAGS: Z 0 H C
			case 0x8E: OutputDisassembledInstruction("ADC A [HL]", pc, opcode, 1); break; // "ADC A [HL]" B:1 C:8 FLAGS: Z 0 H C
			case 0x8F: OutputDisassembledInstruction("ADC A A", pc, opcode, 1); break; // "ADC A A" B:1 C:4 FLAGS: Z 0 H C

			case 0x90: OutputDisassembledInstruction("SUB A B", pc, opcode, 1); break; // "SUB A B" B:1 C:4 FLAGS: Z 1 H C
			case 0x91: OutputDisassembledInstruction("SUB A C", pc, opcode, 1); break; // "SUB A C" B:1 C:4 FLAGS: Z 1 H C
			case 0x92: OutputDisassembledInstruction("SUB A D", pc, opcode, 1); break; // "SUB A D" B:1 C:4 FLAGS: Z 1 H C
			case 0x93: OutputDisassembledInstruction("SUB A E", pc, opcode, 1); break; // "SUB A E" B:1 C:4 FLAGS: Z 1 H C
			case 0x94: OutputDisassembledInstruction("SUB A H", pc, opcode, 1); break; // "SUB A H" B:1 C:4 FLAGS: Z 1 H C
			case 0x95: OutputDisassembledInstruction("SUB A L", pc, opcode, 1); break; // "SUB A L" B:1 C:4 FLAGS: Z 1 H C
			case 0x96: OutputDisassembledInstruction("SUB A [HL]", pc, opcode, 1); break; // "SUB A [HL]" B:1 C:8 FLAGS: Z 1 H C
			case 0x97: OutputDisassembledInstruction("SUB A A", pc, opcode, 1); break; // "SUB A A" B:1 C:4 FLAGS: 1 1 0 0
			case 0x98: OutputDisassembledInstruction("SBC A B", pc, opcode, 1); break; // "SBC A B" B:1 C:4 FLAGS: Z 1 H C
			case 0x99: OutputDisassembledInstruction("SBC A C", pc, opcode, 1); break; // "SBC A C" B:1 C:4 FLAGS: Z 1 H C
			case 0x9A: OutputDisassembledInstruction("SBC A D", pc, opcode, 1); break; // "SBC A D" B:1 C:4 FLAGS: Z 1 H C
			case 0x9B: OutputDisassembledInstruction("SBC A E", pc, opcode, 1); break; // "SBC A E" B:1 C:4 FLAGS: Z 1 H C
			case 0x9C: OutputDisassembledInstruction("SBC A H", pc, opcode, 1); break; // "SBC A H" B:1 C:4 FLAGS: Z 1 H C
			case 0x9D: OutputDisassembledInstruction("SBC A L", pc, opcode, 1); break; // "SBC A L" B:1 C:4 FLAGS: Z 1 H C
			case 0x9E: OutputDisassembledInstruction("SBC A [HL]", pc, opcode, 1); break; // "SBC A [HL]" B:1 C:8 FLAGS: Z 1 H C
			case 0x9F: OutputDisassembledInstruction("SBC A A", pc, opcode, 1); break; // "SBC A A" B:1 C:4 FLAGS: Z 1 H -

			case 0xA0: OutputDisassembledInstruction("AND A B", pc, opcode, 1); break; // "AND A B" B:1 C:4 FLAGS: Z 0 1 0
			case 0xA1: OutputDisassembledInstruction("AND A C", pc, opcode, 1); break; // "AND A C" B:1 C:4 FLAGS: Z 0 1 0
			case 0xA2: OutputDisassembledInstruction("AND A D", pc, opcode, 1); break; // "AND A D" B:1 C:4 FLAGS: Z 0 1 0
			case 0xA3: OutputDisassembledInstruction("AND A E", pc, opcode, 1); break; // "AND A E" B:1 C:4 FLAGS: Z 0 1 0
			case 0xA4: OutputDisassembledInstruction("AND A H", pc, opcode, 1); break; // "AND A H" B:1 C:4 FLAGS: Z 0 1 0
			case 0xA5: OutputDisassembledInstruction("AND A L", pc, opcode, 1); break; // "AND A L" B:1 C:4 FLAGS: Z 0 1 0
			case 0xA6: OutputDisassembledInstruction("AND A [HL]", pc, opcode, 1); break; // "AND A [HL]" B:1 C:8 FLAGS: Z 0 1 0
			case 0xA7: OutputDisassembledInstruction("AND A A", pc, opcode, 1); break; // "AND A A" B:1 C:4 FLAGS: Z 0 1 0
			case 0xA8: OutputDisassembledInstruction("XOR A B", pc, opcode, 1); break; // "XOR A B" B:1 C:4 FLAGS: Z 0 0 0
			case 0xA9: OutputDisassembledInstruction("XOR A C", pc, opcode, 1); break; // "XOR A C" B:1 C:4 FLAGS: Z 0 0 0
			case 0xAA: OutputDisassembledInstruction("XOR A D", pc, opcode, 1); break; // "XOR A D" B:1 C:4 FLAGS: Z 0 0 0
			case 0xAB: OutputDisassembledInstruction("XOR A E", pc, opcode, 1); break; // "XOR A E" B:1 C:4 FLAGS: Z 0 0 0
			case 0xAC: OutputDisassembledInstruction("XOR A H", pc, opcode, 1); break; // "XOR A H" B:1 C:4 FLAGS: Z 0 0 0
			case 0xAD: OutputDisassembledInstruction("XOR A L", pc, opcode, 1); break; // "XOR A L" B:1 C:4 FLAGS: Z 0 0 0
			case 0xAE: OutputDisassembledInstruction("XOR A [HL]", pc, opcode, 1); break; // "XOR A [HL]" B:1 C:8 FLAGS: Z 0 0 0
			case 0xAF: OutputDisassembledInstruction("XOR A A", pc, opcode, 1); break; // "XOR A A" B:1 C:4 FLAGS: 1 0 0 0

			case 0xB0: OutputDisassembledInstruction("OR A B", pc, opcode, 1); break; // "OR A B" B:1 C:4 FLAGS: Z 0 0 0
			case 0xB1: OutputDisassembledInstruction("OR A C", pc, opcode, 1); break; // "OR A C" B:1 C:4 FLAGS: Z 0 0 0
			case 0xB2: OutputDisassembledInstruction("OR A D", pc, opcode, 1); break; // "OR A D" B:1 C:4 FLAGS: Z 0 0 0
			case 0xB3: OutputDisassembledInstruction("OR A E", pc, opcode, 1); break; // "OR A E" B:1 C:4 FLAGS: Z 0 0 0
			case 0xB4: OutputDisassembledInstruction("OR A H", pc, opcode, 1); break; // "OR A H" B:1 C:4 FLAGS: Z 0 0 0
			case 0xB5: OutputDisassembledInstruction("OR A L", pc, opcode, 1); break; // "OR A L" B:1 C:4 FLAGS: Z 0 0 0
			case 0xB6: OutputDisassembledInstruction("OR A [HL]", pc, opcode, 1); break; // "OR A [HL]" B:1 C:8 FLAGS: Z 0 0 0
			case 0xB7: OutputDisassembledInstruction("OR A A", pc, opcode, 1); break; // "OR A A" B:1 C:4 FLAGS: Z 0 0 0
			case 0xB8: OutputDisassembledInstruction("CP A B", pc, opcode, 1); break; // "CP A B" B:1 C:4 FLAGS: Z 1 H C
			case 0xB9: OutputDisassembledInstruction("CP A C", pc, opcode, 1); break; // "CP A C" B:1 C:4 FLAGS: Z 1 H C
			case 0xBA: OutputDisassembledInstruction("CP A D", pc, opcode, 1); break; // "CP A D" B:1 C:4 FLAGS: Z 1 H C
			case 0xBB: OutputDisassembledInstruction("CP A E", pc, opcode, 1); break; // "CP A E" B:1 C:4 FLAGS: Z 1 H C
			case 0xBC: OutputDisassembledInstruction("CP A H", pc, opcode, 1); break; // "CP A H" B:1 C:4 FLAGS: Z 1 H C
			case 0xBD: OutputDisassembledInstruction("CP A L", pc, opcode, 1); break; // "CP A L" B:1 C:4 FLAGS: Z 1 H C
			case 0xBE: OutputDisassembledInstruction("CP A [HL]", pc, opcode, 1); break; // "CP A [HL]" B:1 C:8 FLAGS: Z 1 H C
			case 0xBF: OutputDisassembledInstruction("CP A A", pc, opcode, 1); break; // "CP A A" B:1 C:4 FLAGS: 1 1 0 0

			case 0xC0: OutputDisassembledInstruction("RET NZ", pc, opcode, 1); break; // "RET NZ" B:1 C:208 FLAGS: - - - -
			case 0xC1: OutputDisassembledInstruction("POP BC", pc, opcode, 1); break; // "POP BC" B:1 C:12 FLAGS: - - - -
			case 0xC2: OutputDisassembledInstruction("JP NZ a16", pc, opcode, 3); opBytes = 3; break; // "JP NZ a16" B:3 C:1612 FLAGS: - - - -
			case 0xC3: OutputDisassembledInstruction("JP a16", pc, opcode, 3); opBytes = 3; break; // "JP a16" B:3 C:16 FLAGS: - - - -
			case 0xC4: OutputDisassembledInstruction("CALL NZ a16", pc, opcode, 3); opBytes = 3; break; // "CALL NZ a16" B:3 C:2412 FLAGS: - - - -
			case 0xC5: OutputDisassembledInstruction("PUSH BC", pc, opcode, 1); break; // "PUSH BC" B:1 C:16 FLAGS: - - - -
			case 0xC6: OutputDisassembledInstruction("ADD A n8", pc, opcode, 2); opBytes = 2; break; // "ADD A n8" B:2 C:8 FLAGS: Z 0 H C
			case 0xC7: OutputDisassembledInstruction("RST $00", pc, opcode, 1); break; // "RST $00" B:1 C:16 FLAGS: - - - -
			case 0xC8: OutputDisassembledInstruction("RET Z", pc, opcode, 1); break; // "RET Z" B:1 C:208 FLAGS: - - - -
			case 0xC9: OutputDisassembledInstruction("RET", pc, opcode, 1); break; // "RET" B:1 C:16 FLAGS: - - - -
			case 0xCA: OutputDisassembledInstruction("JP Z a16", pc, opcode, 3); opBytes = 3; break; // "JP Z a16" B:3 C:1612 FLAGS: - - - -

				// "PREFIX" B:1 C:4 FLAGS: - - - -
				// Address CB triggers a 16 bit opcode 
			case 0xCB:
			{
				Disasseble16bit(opcode, pc);
				opBytes = 2;
				break;
			}

			case 0xCC: OutputDisassembledInstruction("CALL Z a16", pc, opcode, 3); opBytes = 3; break; // "CALL Z a16" B:3 C:2412 FLAGS: - - - -
			case 0xCD: OutputDisassembledInstruction("CALL a16", pc, opcode, 3); opBytes = 3; break; // "CALL a16" B:3 C:24 FLAGS: - - - -
			case 0xCE: OutputDisassembledInstruction("ADC A n8", pc, opcode, 2); opBytes = 2; break; // "ADC A n8" B:2 C:8 FLAGS: Z 0 H C
			case 0xCF: OutputDisassembledInstruction("RST $08", pc, opcode, 1); break; // "RST $08" B:1 C:16 FLAGS: - - - -

			case 0xD0: OutputDisassembledInstruction("RET NC", pc, opcode, 1); break; // "RET NC" B:1 C:208 FLAGS: - - - -
			case 0xD1: OutputDisassembledInstruction("POP DE", pc, opcode, 1); break; // "POP DE" B:1 C:12 FLAGS: - - - -
			case 0xD2: OutputDisassembledInstruction("JP NC a16", pc, opcode, 3); opBytes = 3; break; // "JP NC a16" B:3 C:1612 FLAGS: - - - -
			case 0xD3: OutputDisassembledInstruction("ILLEGAL_D3", pc, opcode, 1); break; // "ILLEGAL_D3" B:1 C:4 FLAGS: - - - -
			case 0xD4: OutputDisassembledInstruction("CALL NC a16", pc, opcode, 3); opBytes = 3; break; // "CALL NC a16" B:3 C:2412 FLAGS: - - - -
			case 0xD5: OutputDisassembledInstruction("PUSH DE", pc, opcode, 1); break; // "PUSH DE" B:1 C:16 FLAGS: - - - -
			case 0xD6: OutputDisassembledInstruction("SUB A n8", pc, opcode, 2); opBytes = 2; break; // "SUB A n8" B:2 C:8 FLAGS: Z 1 H C
			case 0xD7: OutputDisassembledInstruction("RST $10", pc, opcode, 1); break; // "RST $10" B:1 C:16 FLAGS: - - - -
			case 0xD8: OutputDisassembledInstruction("RET C", pc, opcode, 1); break; // "RET C" B:1 C:208 FLAGS: - - - -
			case 0xD9: OutputDisassembledInstruction("RETI", pc, opcode, 1); break; // "RETI" B:1 C:16 FLAGS: - - - -
			case 0xDA: OutputDisassembledInstruction("JP C a16", pc, opcode, 3); opBytes = 3; break; // "JP C a16" B:3 C:1612 FLAGS: - - - -
			case 0xDB: OutputDisassembledInstruction("ILLEGAL_DB", pc, opcode, 1); break; // "ILLEGAL_DB" B:1 C:4 FLAGS: - - - -
			case 0xDC: OutputDisassembledInstruction("CALL C a16", pc, opcode, 3); opBytes = 3; break; // "CALL C a16" B:3 C:2412 FLAGS: - - - -
			case 0xDD: OutputDisassembledInstruction("ILLEGAL_DD", pc, opcode, 1); break; // "ILLEGAL_DD" B:1 C:4 FLAGS: - - - -
			case 0xDE: OutputDisassembledInstruction("SBC A n8", pc, opcode, 2); opBytes = 2; break; // "SBC A n8" B:2 C:8 FLAGS: Z 1 H C
			case 0xDF: OutputDisassembledInstruction("RST $18", pc, opcode, 1); break; // "RST $18" B:1 C:16 FLAGS: - - - -

			case 0xE0: OutputDisassembledInstruction("LDH [a8] A", pc, opcode, 2); opBytes = 2; break; // "LDH [a8] A" B:2 C:12 FLAGS: - - - -
			case 0xE1: OutputDisassembledInstruction("POP HL", pc, opcode, 1); break; // "POP HL" B:1 C:12 FLAGS: - - - -
			case 0xE2: OutputDisassembledInstruction("LD [C] A", pc, opcode, 1); break; // "LD [C] A" B:1 C:8 FLAGS: - - - -
			case 0xE3: OutputDisassembledInstruction("ILLEGAL_E3", pc, opcode, 1); break; // "ILLEGAL_E3" B:1 C:4 FLAGS: - - - -
			case 0xE4: OutputDisassembledInstruction("ILLEGAL_E4", pc, opcode, 1); break; // "ILLEGAL_E4" B:1 C:4 FLAGS: - - - -
			case 0xE5: OutputDisassembledInstruction("PUSH HL", pc, opcode, 1); break; // "PUSH HL" B:1 C:16 FLAGS: - - - -
			case 0xE6: OutputDisassembledInstruction("AND A n8", pc, opcode, 2); opBytes = 2; break; // "AND A n8" B:2 C:8 FLAGS: Z 0 1 0
			case 0xE7: OutputDisassembledInstruction("RST $20", pc, opcode, 1); break; // "RST $20" B:1 C:16 FLAGS: - - - -
			case 0xE8: OutputDisassembledInstruction("ADD SP e8", pc, opcode, 2); opBytes = 2; break; // "ADD SP e8" B:2 C:16 FLAGS: 0 0 H C
			case 0xE9: OutputDisassembledInstruction("JP HL", pc, opcode, 1); break; // "JP HL" B:1 C:4 FLAGS: - - - -
			case 0xEA: OutputDisassembledInstruction("LD [a16] A", pc, opcode, 3); opBytes = 3; break; // "LD [a16] A" B:3 C:16 FLAGS: - - - -
			case 0xEB: OutputDisassembledInstruction("ILLEGAL_EB", pc, opcode, 1); break; // "ILLEGAL_EB" B:1 C:4 FLAGS: - - - -
			case 0xEC: OutputDisassembledInstruction("ILLEGAL_EC", pc, opcode, 1); break; // "ILLEGAL_EC" B:1 C:4 FLAGS: - - - -
			case 0xED: OutputDisassembledInstruction("ILLEGAL_ED", pc, opcode, 1); break; // "ILLEGAL_ED" B:1 C:4 FLAGS: - - - -
			case 0xEE: OutputDisassembledInstruction("XOR A n8", pc, opcode, 2); opBytes = 2; break; // "XOR A n8" B:2 C:8 FLAGS: Z 0 0 0
			case 0xEF: OutputDisassembledInstruction("RST $28", pc, opcode, 1); break; // "RST $28" B:1 C:16 FLAGS: - - - -

			case 0xF0: OutputDisassembledInstruction("LDH A [a8]", pc, opcode, 2); opBytes = 2; break; // "LDH A [a8]" B:2 C:12 FLAGS: - - - -
			case 0xF1: OutputDisassembledInstruction("POP AF", pc, opcode, 1); break; // "POP AF" B:1 C:12 FLAGS: Z N H C
			case 0xF2: OutputDisassembledInstruction("LD A [C]", pc, opcode, 1); break; // "LD A [C]" B:1 C:8 FLAGS: - - - -
			case 0xF3: OutputDisassembledInstruction("DI", pc, opcode, 1); break; // "DI" B:1 C:4 FLAGS: - - - -
			case 0xF4: OutputDisassembledInstruction("ILLEGAL_F4", pc, opcode, 1); break; // "ILLEGAL_F4" B:1 C:4 FLAGS: - - - -
			case 0xF5: OutputDisassembledInstruction("PUSH AF", pc, opcode, 1); break; // "PUSH AF" B:1 C:16 FLAGS: - - - -
			case 0xF6: OutputDisassembledInstruction("OR A n8", pc, opcode, 2); opBytes = 2; break; // "OR A n8" B:2 C:8 FLAGS: Z 0 0 0
			case 0xF7: OutputDisassembledInstruction("RST $30", pc, opcode, 1); break; // "RST $30" B:1 C:16 FLAGS: - - - -
			case 0xF8: OutputDisassembledInstruction("LD HL SP e8", pc, opcode, 2); opBytes = 2; break; // "LD HL SP e8" B:2 C:12 FLAGS: 0 0 H C
			case 0xF9: OutputDisassembledInstruction("LD SP HL", pc, opcode, 1); break; // "LD SP HL" B:1 C:8 FLAGS: - - - -
			case 0xFA: OutputDisassembledInstruction("LD A [a16]", pc, opcode, 3); opBytes = 3; break; // "LD A [a16]" B:3 C:16 FLAGS: - - - -
			case 0xFB: OutputDisassembledInstruction("EI", pc, opcode, 1); break; // "EI" B:1 C:4 FLAGS: - - - -
			case 0xFC: OutputDisassembledInstruction("ILLEGAL_FC", pc, opcode, 1); break; // "ILLEGAL_FC" B:1 C:4 FLAGS: - - - -
			case 0xFD: OutputDisassembledInstruction("ILLEGAL_FD", pc, opcode, 1); break; // "ILLEGAL_FD" B:1 C:4 FLAGS: - - - -
			case 0xFE: OutputDisassembledInstruction("CP A n8", pc, opcode, 2); opBytes = 2; break; // "CP A n8" B:2 C:8 FLAGS: Z 1 H C
			case 0xFF: OutputDisassembledInstruction("RST $38", pc, opcode, 1); break; // "RST $38" B:1 C:16 FLAGS: - - - -

			default: OutputDisassembledInstruction("", pc, opcode, 1); break;
		}

		return opBytes;
	}

	std::map<uint16_t, std::string> Cpu::DisassebleAll()
	{
		std::map<uint16_t, std::string> mapLines;
		uint16_t pc = 0;

		while (pc < 0x7FFF)
		{
			uint8_t* opcode = &m_MMU.Read(pc);
			int nextPC = Disassemble(opcode, pc);
			mapLines[pc] = GetCurrentInstruction();
			pc += nextPC;
		}

		return mapLines;
	}

	void Cpu::Disasseble16bit(uint8_t* opcode, int pc)
	{
		// Note: All 16 bit opcodes are 2 bytes.
		uint16_t opcode16bit = (opcode[0] << 8) | opcode[1];

		switch (opcode16bit)
		{
			case 0xCB00: OutputDisassembledInstruction("RLC B", pc, opcode, 2); break; // "RLC B" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB01: OutputDisassembledInstruction("RLC C", pc, opcode, 2); break; // "RLC C" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB02: OutputDisassembledInstruction("RLC D", pc, opcode, 2); break; // "RLC D" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB03: OutputDisassembledInstruction("RLC E", pc, opcode, 2); break; // "RLC E" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB04: OutputDisassembledInstruction("RLC H", pc, opcode, 2); break; // "RLC H" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB05: OutputDisassembledInstruction("RLC L", pc, opcode, 2); break; // "RLC L" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB06: OutputDisassembledInstruction("RLC [HL]", pc, opcode, 2); break; // "RLC [HL]" B:2 C:16 FLAGS: Z 0 0 C
			case 0xCB07: OutputDisassembledInstruction("RLC A", pc, opcode, 2); break; // "RLC A" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB08: OutputDisassembledInstruction("RRC B", pc, opcode, 2); break; // "RRC B" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB09: OutputDisassembledInstruction("RRC C", pc, opcode, 2); break; // "RRC C" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB0A: OutputDisassembledInstruction("RRC D", pc, opcode, 2); break; // "RRC D" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB0B: OutputDisassembledInstruction("RRC E", pc, opcode, 2); break; // "RRC E" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB0C: OutputDisassembledInstruction("RRC H", pc, opcode, 2); break; // "RRC H" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB0D: OutputDisassembledInstruction("RRC L", pc, opcode, 2); break; // "RRC L" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB0E: OutputDisassembledInstruction("RRC [HL]", pc, opcode, 2); break; // "RRC [HL]" B:2 C:16 FLAGS: Z 0 0 C
			case 0xCB0F: OutputDisassembledInstruction("RRC A", pc, opcode, 2); break; // "RRC A" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB10: OutputDisassembledInstruction("RL B", pc, opcode, 2); break; // "RL B" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB11: OutputDisassembledInstruction("RL C", pc, opcode, 2); break; // "RL C" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB12: OutputDisassembledInstruction("RL D", pc, opcode, 2); break; // "RL D" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB13: OutputDisassembledInstruction("RL E", pc, opcode, 2); break; // "RL E" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB14: OutputDisassembledInstruction("RL H", pc, opcode, 2); break; // "RL H" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB15: OutputDisassembledInstruction("RL L", pc, opcode, 2); break; // "RL L" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB16: OutputDisassembledInstruction("RL [HL]", pc, opcode, 2); break; // "RL [HL]" B:2 C:16 FLAGS: Z 0 0 C
			case 0xCB17: OutputDisassembledInstruction("RL A", pc, opcode, 2); break; // "RL A" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB18: OutputDisassembledInstruction("RR B", pc, opcode, 2); break; // "RR B" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB19: OutputDisassembledInstruction("RR C", pc, opcode, 2); break; // "RR C" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB1A: OutputDisassembledInstruction("RR D", pc, opcode, 2); break; // "RR D" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB1B: OutputDisassembledInstruction("RR E", pc, opcode, 2); break; // "RR E" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB1C: OutputDisassembledInstruction("RR H", pc, opcode, 2); break; // "RR H" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB1D: OutputDisassembledInstruction("RR L", pc, opcode, 2); break; // "RR L" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB1E: OutputDisassembledInstruction("RR [HL]", pc, opcode, 2); break; // "RR [HL]" B:2 C:16 FLAGS: Z 0 0 C
			case 0xCB1F: OutputDisassembledInstruction("RR A", pc, opcode, 2); break; // "RR A" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB20: OutputDisassembledInstruction("SLA B", pc, opcode, 2); break; // "SLA B" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB21: OutputDisassembledInstruction("SLA C", pc, opcode, 2); break; // "SLA C" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB22: OutputDisassembledInstruction("SLA D", pc, opcode, 2); break; // "SLA D" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB23: OutputDisassembledInstruction("SLA E", pc, opcode, 2); break; // "SLA E" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB24: OutputDisassembledInstruction("SLA H", pc, opcode, 2); break; // "SLA H" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB25: OutputDisassembledInstruction("SLA L", pc, opcode, 2); break; // "SLA L" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB26: OutputDisassembledInstruction("SLA [HL]", pc, opcode, 2); break; // "SLA [HL]" B:2 C:16 FLAGS: Z 0 0 C
			case 0xCB27: OutputDisassembledInstruction("SLA A", pc, opcode, 2); break; // "SLA A" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB28: OutputDisassembledInstruction("SRA B", pc, opcode, 2); break; // "SRA B" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB29: OutputDisassembledInstruction("SRA C", pc, opcode, 2); break; // "SRA C" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB2A: OutputDisassembledInstruction("SRA D", pc, opcode, 2); break; // "SRA D" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB2B: OutputDisassembledInstruction("SRA E", pc, opcode, 2); break; // "SRA E" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB2C: OutputDisassembledInstruction("SRA H", pc, opcode, 2); break; // "SRA H" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB2D: OutputDisassembledInstruction("SRA L", pc, opcode, 2); break; // "SRA L" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB2E: OutputDisassembledInstruction("SRA [HL]", pc, opcode, 2); break; // "SRA [HL]" B:2 C:16 FLAGS: Z 0 0 C
			case 0xCB2F: OutputDisassembledInstruction("SRA A", pc, opcode, 2); break; // "SRA A" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB30: OutputDisassembledInstruction("SWAP B", pc, opcode, 2); break; // "SWAP B" B:2 C:8 FLAGS: Z 0 0 0
			case 0xCB31: OutputDisassembledInstruction("SWAP C", pc, opcode, 2); break; // "SWAP C" B:2 C:8 FLAGS: Z 0 0 0
			case 0xCB32: OutputDisassembledInstruction("SWAP D", pc, opcode, 2); break; // "SWAP D" B:2 C:8 FLAGS: Z 0 0 0
			case 0xCB33: OutputDisassembledInstruction("SWAP E", pc, opcode, 2); break; // "SWAP E" B:2 C:8 FLAGS: Z 0 0 0
			case 0xCB34: OutputDisassembledInstruction("SWAP H", pc, opcode, 2); break; // "SWAP H" B:2 C:8 FLAGS: Z 0 0 0
			case 0xCB35: OutputDisassembledInstruction("SWAP L", pc, opcode, 2); break; // "SWAP L" B:2 C:8 FLAGS: Z 0 0 0
			case 0xCB36: OutputDisassembledInstruction("SWAP [HL]", pc, opcode, 2); break; // "SWAP [HL]" B:2 C:16 FLAGS: Z 0 0 0
			case 0xCB37: OutputDisassembledInstruction("SWAP A", pc, opcode, 2); break; // "SWAP A" B:2 C:8 FLAGS: Z 0 0 0
			case 0xCB38: OutputDisassembledInstruction("SRL B", pc, opcode, 2); break; // "SRL B" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB39: OutputDisassembledInstruction("SRL C", pc, opcode, 2); break; // "SRL C" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB3A: OutputDisassembledInstruction("SRL D", pc, opcode, 2); break; // "SRL D" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB3B: OutputDisassembledInstruction("SRL E", pc, opcode, 2); break; // "SRL E" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB3C: OutputDisassembledInstruction("SRL H", pc, opcode, 2); break; // "SRL H" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB3D: OutputDisassembledInstruction("SRL L", pc, opcode, 2); break; // "SRL L" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB3E: OutputDisassembledInstruction("SRL [HL]", pc, opcode, 2); break; // "SRL [HL]" B:2 C:16 FLAGS: Z 0 0 C
			case 0xCB3F: OutputDisassembledInstruction("SRL A", pc, opcode, 2); break; // "SRL A" B:2 C:8 FLAGS: Z 0 0 C
			case 0xCB40: OutputDisassembledInstruction("BIT 0 B", pc, opcode, 2); break; // "BIT 0 B" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB41: OutputDisassembledInstruction("BIT 0 C", pc, opcode, 2); break; // "BIT 0 C" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB42: OutputDisassembledInstruction("BIT 0 D", pc, opcode, 2); break; // "BIT 0 D" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB43: OutputDisassembledInstruction("BIT 0 E", pc, opcode, 2); break; // "BIT 0 E" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB44: OutputDisassembledInstruction("BIT 0 H", pc, opcode, 2); break; // "BIT 0 H" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB45: OutputDisassembledInstruction("BIT 0 L", pc, opcode, 2); break; // "BIT 0 L" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB46: OutputDisassembledInstruction("BIT 0 [HL]", pc, opcode, 2); break; // "BIT 0 [HL]" B:2 C:12 FLAGS: Z 0 1 -
			case 0xCB47: OutputDisassembledInstruction("BIT 0 A", pc, opcode, 2); break; // "BIT 0 A" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB48: OutputDisassembledInstruction("BIT 1 B", pc, opcode, 2); break; // "BIT 1 B" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB49: OutputDisassembledInstruction("BIT 1 C", pc, opcode, 2); break; // "BIT 1 C" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB4A: OutputDisassembledInstruction("BIT 1 D", pc, opcode, 2); break; // "BIT 1 D" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB4B: OutputDisassembledInstruction("BIT 1 E", pc, opcode, 2); break; // "BIT 1 E" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB4C: OutputDisassembledInstruction("BIT 1 H", pc, opcode, 2); break; // "BIT 1 H" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB4D: OutputDisassembledInstruction("BIT 1 L", pc, opcode, 2); break; // "BIT 1 L" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB4E: OutputDisassembledInstruction("BIT 1 [HL]", pc, opcode, 2); break; // "BIT 1 [HL]" B:2 C:12 FLAGS: Z 0 1 -
			case 0xCB4F: OutputDisassembledInstruction("BIT 1 A", pc, opcode, 2); break; // "BIT 1 A" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB50: OutputDisassembledInstruction("BIT 2 B", pc, opcode, 2); break; // "BIT 2 B" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB51: OutputDisassembledInstruction("BIT 2 C", pc, opcode, 2); break; // "BIT 2 C" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB52: OutputDisassembledInstruction("BIT 2 D", pc, opcode, 2); break; // "BIT 2 D" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB53: OutputDisassembledInstruction("BIT 2 E", pc, opcode, 2); break; // "BIT 2 E" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB54: OutputDisassembledInstruction("BIT 2 H", pc, opcode, 2); break; // "BIT 2 H" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB55: OutputDisassembledInstruction("BIT 2 L", pc, opcode, 2); break; // "BIT 2 L" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB56: OutputDisassembledInstruction("BIT 2 [HL]", pc, opcode, 2); break; // "BIT 2 [HL]" B:2 C:12 FLAGS: Z 0 1 -
			case 0xCB57: OutputDisassembledInstruction("BIT 2 A", pc, opcode, 2); break; // "BIT 2 A" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB58: OutputDisassembledInstruction("BIT 3 B", pc, opcode, 2); break; // "BIT 3 B" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB59: OutputDisassembledInstruction("BIT 3 C", pc, opcode, 2); break; // "BIT 3 C" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB5A: OutputDisassembledInstruction("BIT 3 D", pc, opcode, 2); break; // "BIT 3 D" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB5B: OutputDisassembledInstruction("BIT 3 E", pc, opcode, 2); break; // "BIT 3 E" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB5C: OutputDisassembledInstruction("BIT 3 H", pc, opcode, 2); break; // "BIT 3 H" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB5D: OutputDisassembledInstruction("BIT 3 L", pc, opcode, 2); break; // "BIT 3 L" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB5E: OutputDisassembledInstruction("BIT 3 [HL]", pc, opcode, 2); break; // "BIT 3 [HL]" B:2 C:12 FLAGS: Z 0 1 -
			case 0xCB5F: OutputDisassembledInstruction("BIT 3 A", pc, opcode, 2); break; // "BIT 3 A" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB60: OutputDisassembledInstruction("BIT 4 B", pc, opcode, 2); break; // "BIT 4 B" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB61: OutputDisassembledInstruction("BIT 4 C", pc, opcode, 2); break; // "BIT 4 C" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB62: OutputDisassembledInstruction("BIT 4 D", pc, opcode, 2); break; // "BIT 4 D" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB63: OutputDisassembledInstruction("BIT 4 E", pc, opcode, 2); break; // "BIT 4 E" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB64: OutputDisassembledInstruction("BIT 4 H", pc, opcode, 2); break; // "BIT 4 H" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB65: OutputDisassembledInstruction("BIT 4 L", pc, opcode, 2); break; // "BIT 4 L" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB66: OutputDisassembledInstruction("BIT 4 [HL]", pc, opcode, 2); break; // "BIT 4 [HL]" B:2 C:12 FLAGS: Z 0 1 -
			case 0xCB67: OutputDisassembledInstruction("BIT 4 A", pc, opcode, 2); break; // "BIT 4 A" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB68: OutputDisassembledInstruction("BIT 5 B", pc, opcode, 2); break; // "BIT 5 B" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB69: OutputDisassembledInstruction("BIT 5 C", pc, opcode, 2); break; // "BIT 5 C" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB6A: OutputDisassembledInstruction("BIT 5 D", pc, opcode, 2); break; // "BIT 5 D" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB6B: OutputDisassembledInstruction("BIT 5 E", pc, opcode, 2); break; // "BIT 5 E" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB6C: OutputDisassembledInstruction("BIT 5 H", pc, opcode, 2); break; // "BIT 5 H" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB6D: OutputDisassembledInstruction("BIT 5 L", pc, opcode, 2); break; // "BIT 5 L" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB6E: OutputDisassembledInstruction("BIT 5 [HL]", pc, opcode, 2); break; // "BIT 5 [HL]" B:2 C:12 FLAGS: Z 0 1 -
			case 0xCB6F: OutputDisassembledInstruction("BIT 5 A", pc, opcode, 2); break; // "BIT 5 A" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB70: OutputDisassembledInstruction("BIT 6 B", pc, opcode, 2); break; // "BIT 6 B" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB71: OutputDisassembledInstruction("BIT 6 C", pc, opcode, 2); break; // "BIT 6 C" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB72: OutputDisassembledInstruction("BIT 6 D", pc, opcode, 2); break; // "BIT 6 D" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB73: OutputDisassembledInstruction("BIT 6 E", pc, opcode, 2); break; // "BIT 6 E" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB74: OutputDisassembledInstruction("BIT 6 H", pc, opcode, 2); break; // "BIT 6 H" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB75: OutputDisassembledInstruction("BIT 6 L", pc, opcode, 2); break; // "BIT 6 L" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB76: OutputDisassembledInstruction("BIT 6 [HL]", pc, opcode, 2); break; // "BIT 6 [HL]" B:2 C:12 FLAGS: Z 0 1 -
			case 0xCB77: OutputDisassembledInstruction("BIT 6 A", pc, opcode, 2); break; // "BIT 6 A" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB78: OutputDisassembledInstruction("BIT 7 B", pc, opcode, 2); break; // "BIT 7 B" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB79: OutputDisassembledInstruction("BIT 7 C", pc, opcode, 2); break; // "BIT 7 C" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB7A: OutputDisassembledInstruction("BIT 7 D", pc, opcode, 2); break; // "BIT 7 D" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB7B: OutputDisassembledInstruction("BIT 7 E", pc, opcode, 2); break; // "BIT 7 E" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB7C: OutputDisassembledInstruction("BIT 7 H", pc, opcode, 2); break; // "BIT 7 H" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB7D: OutputDisassembledInstruction("BIT 7 L", pc, opcode, 2); break; // "BIT 7 L" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB7E: OutputDisassembledInstruction("BIT 7 [HL]", pc, opcode, 2); break; // "BIT 7 [HL]" B:2 C:12 FLAGS: Z 0 1 -
			case 0xCB7F: OutputDisassembledInstruction("BIT 7 A", pc, opcode, 2); break; // "BIT 7 A" B:2 C:8 FLAGS: Z 0 1 -
			case 0xCB80: OutputDisassembledInstruction("RES 0 B", pc, opcode, 2); break; // "RES 0 B" B:2 C:8 FLAGS: - - - -
			case 0xCB81: OutputDisassembledInstruction("RES 0 C", pc, opcode, 2); break; // "RES 0 C" B:2 C:8 FLAGS: - - - -
			case 0xCB82: OutputDisassembledInstruction("RES 0 D", pc, opcode, 2); break; // "RES 0 D" B:2 C:8 FLAGS: - - - -
			case 0xCB83: OutputDisassembledInstruction("RES 0 E", pc, opcode, 2); break; // "RES 0 E" B:2 C:8 FLAGS: - - - -
			case 0xCB84: OutputDisassembledInstruction("RES 0 H", pc, opcode, 2); break; // "RES 0 H" B:2 C:8 FLAGS: - - - -
			case 0xCB85: OutputDisassembledInstruction("RES 0 L", pc, opcode, 2); break; // "RES 0 L" B:2 C:8 FLAGS: - - - -
			case 0xCB86: OutputDisassembledInstruction("RES 0 [HL]", pc, opcode, 2); break; // "RES 0 [HL]" B:2 C:16 FLAGS: - - - -
			case 0xCB87: OutputDisassembledInstruction("RES 0 A", pc, opcode, 2); break; // "RES 0 A" B:2 C:8 FLAGS: - - - -
			case 0xCB88: OutputDisassembledInstruction("RES 1 B", pc, opcode, 2); break; // "RES 1 B" B:2 C:8 FLAGS: - - - -
			case 0xCB89: OutputDisassembledInstruction("RES 1 C", pc, opcode, 2); break; // "RES 1 C" B:2 C:8 FLAGS: - - - -
			case 0xCB8A: OutputDisassembledInstruction("RES 1 D", pc, opcode, 2); break; // "RES 1 D" B:2 C:8 FLAGS: - - - -
			case 0xCB8B: OutputDisassembledInstruction("RES 1 E", pc, opcode, 2); break; // "RES 1 E" B:2 C:8 FLAGS: - - - -
			case 0xCB8C: OutputDisassembledInstruction("RES 1 H", pc, opcode, 2); break; // "RES 1 H" B:2 C:8 FLAGS: - - - -
			case 0xCB8D: OutputDisassembledInstruction("RES 1 L", pc, opcode, 2); break; // "RES 1 L" B:2 C:8 FLAGS: - - - -
			case 0xCB8E: OutputDisassembledInstruction("RES 1 [HL]", pc, opcode, 2); break; // "RES 1 [HL]" B:2 C:16 FLAGS: - - - -
			case 0xCB8F: OutputDisassembledInstruction("RES 1 A", pc, opcode, 2); break; // "RES 1 A" B:2 C:8 FLAGS: - - - -
			case 0xCB90: OutputDisassembledInstruction("RES 2 B", pc, opcode, 2); break; // "RES 2 B" B:2 C:8 FLAGS: - - - -
			case 0xCB91: OutputDisassembledInstruction("RES 2 C", pc, opcode, 2); break; // "RES 2 C" B:2 C:8 FLAGS: - - - -
			case 0xCB92: OutputDisassembledInstruction("RES 2 D", pc, opcode, 2); break; // "RES 2 D" B:2 C:8 FLAGS: - - - -
			case 0xCB93: OutputDisassembledInstruction("RES 2 E", pc, opcode, 2); break; // "RES 2 E" B:2 C:8 FLAGS: - - - -
			case 0xCB94: OutputDisassembledInstruction("RES 2 H", pc, opcode, 2); break; // "RES 2 H" B:2 C:8 FLAGS: - - - -
			case 0xCB95: OutputDisassembledInstruction("RES 2 L", pc, opcode, 2); break; // "RES 2 L" B:2 C:8 FLAGS: - - - -
			case 0xCB96: OutputDisassembledInstruction("RES 2 [HL]", pc, opcode, 2); break; // "RES 2 [HL]" B:2 C:16 FLAGS: - - - -
			case 0xCB97: OutputDisassembledInstruction("RES 2 A", pc, opcode, 2); break; // "RES 2 A" B:2 C:8 FLAGS: - - - -
			case 0xCB98: OutputDisassembledInstruction("RES 3 B", pc, opcode, 2); break; // "RES 3 B" B:2 C:8 FLAGS: - - - -
			case 0xCB99: OutputDisassembledInstruction("RES 3 C", pc, opcode, 2); break; // "RES 3 C" B:2 C:8 FLAGS: - - - -
			case 0xCB9A: OutputDisassembledInstruction("RES 3 D", pc, opcode, 2); break; // "RES 3 D" B:2 C:8 FLAGS: - - - -
			case 0xCB9B: OutputDisassembledInstruction("RES 3 E", pc, opcode, 2); break; // "RES 3 E" B:2 C:8 FLAGS: - - - -
			case 0xCB9C: OutputDisassembledInstruction("RES 3 H", pc, opcode, 2); break; // "RES 3 H" B:2 C:8 FLAGS: - - - -
			case 0xCB9D: OutputDisassembledInstruction("RES 3 L", pc, opcode, 2); break; // "RES 3 L" B:2 C:8 FLAGS: - - - -
			case 0xCB9E: OutputDisassembledInstruction("RES 3 [HL]", pc, opcode, 2); break; // "RES 3 [HL]" B:2 C:16 FLAGS: - - - -
			case 0xCB9F: OutputDisassembledInstruction("RES 3 A", pc, opcode, 2); break; // "RES 3 A" B:2 C:8 FLAGS: - - - -
			case 0xCBA0: OutputDisassembledInstruction("RES 4 B", pc, opcode, 2); break; // "RES 4 B" B:2 C:8 FLAGS: - - - -
			case 0xCBA1: OutputDisassembledInstruction("RES 4 C", pc, opcode, 2); break; // "RES 4 C" B:2 C:8 FLAGS: - - - -
			case 0xCBA2: OutputDisassembledInstruction("RES 4 D", pc, opcode, 2); break; // "RES 4 D" B:2 C:8 FLAGS: - - - -
			case 0xCBA3: OutputDisassembledInstruction("RES 4 E", pc, opcode, 2); break; // "RES 4 E" B:2 C:8 FLAGS: - - - -
			case 0xCBA4: OutputDisassembledInstruction("RES 4 H", pc, opcode, 2); break; // "RES 4 H" B:2 C:8 FLAGS: - - - -
			case 0xCBA5: OutputDisassembledInstruction("RES 4 L", pc, opcode, 2); break; // "RES 4 L" B:2 C:8 FLAGS: - - - -
			case 0xCBA6: OutputDisassembledInstruction("RES 4 [HL]", pc, opcode, 2); break; // "RES 4 [HL]" B:2 C:16 FLAGS: - - - -
			case 0xCBA7: OutputDisassembledInstruction("RES 4 A", pc, opcode, 2); break; // "RES 4 A" B:2 C:8 FLAGS: - - - -
			case 0xCBA8: OutputDisassembledInstruction("RES 5 B", pc, opcode, 2); break; // "RES 5 B" B:2 C:8 FLAGS: - - - -
			case 0xCBA9: OutputDisassembledInstruction("RES 5 C", pc, opcode, 2); break; // "RES 5 C" B:2 C:8 FLAGS: - - - -
			case 0xCBAA: OutputDisassembledInstruction("RES 5 D", pc, opcode, 2); break; // "RES 5 D" B:2 C:8 FLAGS: - - - -
			case 0xCBAB: OutputDisassembledInstruction("RES 5 E", pc, opcode, 2); break; // "RES 5 E" B:2 C:8 FLAGS: - - - -
			case 0xCBAC: OutputDisassembledInstruction("RES 5 H", pc, opcode, 2); break; // "RES 5 H" B:2 C:8 FLAGS: - - - -
			case 0xCBAD: OutputDisassembledInstruction("RES 5 L", pc, opcode, 2); break; // "RES 5 L" B:2 C:8 FLAGS: - - - -
			case 0xCBAE: OutputDisassembledInstruction("RES 5 [HL]", pc, opcode, 2); break; // "RES 5 [HL]" B:2 C:16 FLAGS: - - - -
			case 0xCBAF: OutputDisassembledInstruction("RES 5 A", pc, opcode, 2); break; // "RES 5 A" B:2 C:8 FLAGS: - - - -
			case 0xCBB0: OutputDisassembledInstruction("RES 6 B", pc, opcode, 2); break; // "RES 6 B" B:2 C:8 FLAGS: - - - -
			case 0xCBB1: OutputDisassembledInstruction("RES 6 C", pc, opcode, 2); break; // "RES 6 C" B:2 C:8 FLAGS: - - - -
			case 0xCBB2: OutputDisassembledInstruction("RES 6 D", pc, opcode, 2); break; // "RES 6 D" B:2 C:8 FLAGS: - - - -
			case 0xCBB3: OutputDisassembledInstruction("RES 6 E", pc, opcode, 2); break; // "RES 6 E" B:2 C:8 FLAGS: - - - -
			case 0xCBB4: OutputDisassembledInstruction("RES 6 H", pc, opcode, 2); break; // "RES 6 H" B:2 C:8 FLAGS: - - - -
			case 0xCBB5: OutputDisassembledInstruction("RES 6 L", pc, opcode, 2); break; // "RES 6 L" B:2 C:8 FLAGS: - - - -
			case 0xCBB6: OutputDisassembledInstruction("RES 6 [HL]", pc, opcode, 2); break; // "RES 6 [HL]" B:2 C:16 FLAGS: - - - -
			case 0xCBB7: OutputDisassembledInstruction("RES 6 A", pc, opcode, 2); break; // "RES 6 A" B:2 C:8 FLAGS: - - - -
			case 0xCBB8: OutputDisassembledInstruction("RES 7 B", pc, opcode, 2); break; // "RES 7 B" B:2 C:8 FLAGS: - - - -
			case 0xCBB9: OutputDisassembledInstruction("RES 7 C", pc, opcode, 2); break; // "RES 7 C" B:2 C:8 FLAGS: - - - -
			case 0xCBBA: OutputDisassembledInstruction("RES 7 D", pc, opcode, 2); break; // "RES 7 D" B:2 C:8 FLAGS: - - - -
			case 0xCBBB: OutputDisassembledInstruction("RES 7 E", pc, opcode, 2); break; // "RES 7 E" B:2 C:8 FLAGS: - - - -
			case 0xCBBC: OutputDisassembledInstruction("RES 7 H", pc, opcode, 2); break; // "RES 7 H" B:2 C:8 FLAGS: - - - -
			case 0xCBBD: OutputDisassembledInstruction("RES 7 L", pc, opcode, 2); break; // "RES 7 L" B:2 C:8 FLAGS: - - - -
			case 0xCBBE: OutputDisassembledInstruction("RES 7 [HL]", pc, opcode, 2); break; // "RES 7 [HL]" B:2 C:16 FLAGS: - - - -
			case 0xCBBF: OutputDisassembledInstruction("RES 7 A", pc, opcode, 2); break; // "RES 7 A" B:2 C:8 FLAGS: - - - -
			case 0xCBC0: OutputDisassembledInstruction("SET 0 B", pc, opcode, 2); break; // "SET 0 B" B:2 C:8 FLAGS: - - - -
			case 0xCBC1: OutputDisassembledInstruction("SET 0 C", pc, opcode, 2); break; // "SET 0 C" B:2 C:8 FLAGS: - - - -
			case 0xCBC2: OutputDisassembledInstruction("SET 0 D", pc, opcode, 2); break; // "SET 0 D" B:2 C:8 FLAGS: - - - -
			case 0xCBC3: OutputDisassembledInstruction("SET 0 E", pc, opcode, 2); break; // "SET 0 E" B:2 C:8 FLAGS: - - - -
			case 0xCBC4: OutputDisassembledInstruction("SET 0 H", pc, opcode, 2); break; // "SET 0 H" B:2 C:8 FLAGS: - - - -
			case 0xCBC5: OutputDisassembledInstruction("SET 0 L", pc, opcode, 2); break; // "SET 0 L" B:2 C:8 FLAGS: - - - -
			case 0xCBC6: OutputDisassembledInstruction("SET 0 [HL]", pc, opcode, 2); break; // "SET 0 [HL]" B:2 C:16 FLAGS: - - - -
			case 0xCBC7: OutputDisassembledInstruction("SET 0 A", pc, opcode, 2); break; // "SET 0 A" B:2 C:8 FLAGS: - - - -
			case 0xCBC8: OutputDisassembledInstruction("SET 1 B", pc, opcode, 2); break; // "SET 1 B" B:2 C:8 FLAGS: - - - -
			case 0xCBC9: OutputDisassembledInstruction("SET 1 C", pc, opcode, 2); break; // "SET 1 C" B:2 C:8 FLAGS: - - - -
			case 0xCBCA: OutputDisassembledInstruction("SET 1 D", pc, opcode, 2); break; // "SET 1 D" B:2 C:8 FLAGS: - - - -
			case 0xCBCB: OutputDisassembledInstruction("SET 1 E", pc, opcode, 2); break; // "SET 1 E" B:2 C:8 FLAGS: - - - -
			case 0xCBCC: OutputDisassembledInstruction("SET 1 H", pc, opcode, 2); break; // "SET 1 H" B:2 C:8 FLAGS: - - - -
			case 0xCBCD: OutputDisassembledInstruction("SET 1 L", pc, opcode, 2); break; // "SET 1 L" B:2 C:8 FLAGS: - - - -
			case 0xCBCE: OutputDisassembledInstruction("SET 1 [HL]", pc, opcode, 2); break; // "SET 1 [HL]" B:2 C:16 FLAGS: - - - -
			case 0xCBCF: OutputDisassembledInstruction("SET 1 A", pc, opcode, 2); break; // "SET 1 A" B:2 C:8 FLAGS: - - - -
			case 0xCBD0: OutputDisassembledInstruction("SET 2 B", pc, opcode, 2); break; // "SET 2 B" B:2 C:8 FLAGS: - - - -
			case 0xCBD1: OutputDisassembledInstruction("SET 2 C", pc, opcode, 2); break; // "SET 2 C" B:2 C:8 FLAGS: - - - -
			case 0xCBD2: OutputDisassembledInstruction("SET 2 D", pc, opcode, 2); break; // "SET 2 D" B:2 C:8 FLAGS: - - - -
			case 0xCBD3: OutputDisassembledInstruction("SET 2 E", pc, opcode, 2); break; // "SET 2 E" B:2 C:8 FLAGS: - - - -
			case 0xCBD4: OutputDisassembledInstruction("SET 2 H", pc, opcode, 2); break; // "SET 2 H" B:2 C:8 FLAGS: - - - -
			case 0xCBD5: OutputDisassembledInstruction("SET 2 L", pc, opcode, 2); break; // "SET 2 L" B:2 C:8 FLAGS: - - - -
			case 0xCBD6: OutputDisassembledInstruction("SET 2 [HL]", pc, opcode, 2); break; // "SET 2 [HL]" B:2 C:16 FLAGS: - - - -
			case 0xCBD7: OutputDisassembledInstruction("SET 2 A", pc, opcode, 2); break; // "SET 2 A" B:2 C:8 FLAGS: - - - -
			case 0xCBD8: OutputDisassembledInstruction("SET 3 B", pc, opcode, 2); break; // "SET 3 B" B:2 C:8 FLAGS: - - - -
			case 0xCBD9: OutputDisassembledInstruction("SET 3 C", pc, opcode, 2); break; // "SET 3 C" B:2 C:8 FLAGS: - - - -
			case 0xCBDA: OutputDisassembledInstruction("SET 3 D", pc, opcode, 2); break; // "SET 3 D" B:2 C:8 FLAGS: - - - -
			case 0xCBDB: OutputDisassembledInstruction("SET 3 E", pc, opcode, 2); break; // "SET 3 E" B:2 C:8 FLAGS: - - - -
			case 0xCBDC: OutputDisassembledInstruction("SET 3 H", pc, opcode, 2); break; // "SET 3 H" B:2 C:8 FLAGS: - - - -
			case 0xCBDD: OutputDisassembledInstruction("SET 3 L", pc, opcode, 2); break; // "SET 3 L" B:2 C:8 FLAGS: - - - -
			case 0xCBDE: OutputDisassembledInstruction("SET 3 [HL]", pc, opcode, 2); break; // "SET 3 [HL]" B:2 C:16 FLAGS: - - - -
			case 0xCBDF: OutputDisassembledInstruction("SET 3 A", pc, opcode, 2); break; // "SET 3 A" B:2 C:8 FLAGS: - - - -
			case 0xCBE0: OutputDisassembledInstruction("SET 4 B", pc, opcode, 2); break; // "SET 4 B" B:2 C:8 FLAGS: - - - -
			case 0xCBE1: OutputDisassembledInstruction("SET 4 C", pc, opcode, 2); break; // "SET 4 C" B:2 C:8 FLAGS: - - - -
			case 0xCBE2: OutputDisassembledInstruction("SET 4 D", pc, opcode, 2); break; // "SET 4 D" B:2 C:8 FLAGS: - - - -
			case 0xCBE3: OutputDisassembledInstruction("SET 4 E", pc, opcode, 2); break; // "SET 4 E" B:2 C:8 FLAGS: - - - -
			case 0xCBE4: OutputDisassembledInstruction("SET 4 H", pc, opcode, 2); break; // "SET 4 H" B:2 C:8 FLAGS: - - - -
			case 0xCBE5: OutputDisassembledInstruction("SET 4 L", pc, opcode, 2); break; // "SET 4 L" B:2 C:8 FLAGS: - - - -
			case 0xCBE6: OutputDisassembledInstruction("SET 4 [HL]", pc, opcode, 2); break; // "SET 4 [HL]" B:2 C:16 FLAGS: - - - -
			case 0xCBE7: OutputDisassembledInstruction("SET 4 A", pc, opcode, 2); break; // "SET 4 A" B:2 C:8 FLAGS: - - - -
			case 0xCBE8: OutputDisassembledInstruction("SET 5 B", pc, opcode, 2); break; // "SET 5 B" B:2 C:8 FLAGS: - - - -
			case 0xCBE9: OutputDisassembledInstruction("SET 5 C", pc, opcode, 2); break; // "SET 5 C" B:2 C:8 FLAGS: - - - -
			case 0xCBEA: OutputDisassembledInstruction("SET 5 D", pc, opcode, 2); break; // "SET 5 D" B:2 C:8 FLAGS: - - - -
			case 0xCBEB: OutputDisassembledInstruction("SET 5 E", pc, opcode, 2); break; // "SET 5 E" B:2 C:8 FLAGS: - - - -
			case 0xCBEC: OutputDisassembledInstruction("SET 5 H", pc, opcode, 2); break; // "SET 5 H" B:2 C:8 FLAGS: - - - -
			case 0xCBED: OutputDisassembledInstruction("SET 5 L", pc, opcode, 2); break; // "SET 5 L" B:2 C:8 FLAGS: - - - -
			case 0xCBEE: OutputDisassembledInstruction("SET 5 [HL]", pc, opcode, 2); break; // "SET 5 [HL]" B:2 C:16 FLAGS: - - - -
			case 0xCBEF: OutputDisassembledInstruction("SET 5 A", pc, opcode, 2); break; // "SET 5 A" B:2 C:8 FLAGS: - - - -
			case 0xCBF0: OutputDisassembledInstruction("SET 6 B", pc, opcode, 2); break; // "SET 6 B" B:2 C:8 FLAGS: - - - -
			case 0xCBF1: OutputDisassembledInstruction("SET 6 C", pc, opcode, 2); break; // "SET 6 C" B:2 C:8 FLAGS: - - - -
			case 0xCBF2: OutputDisassembledInstruction("SET 6 D", pc, opcode, 2); break; // "SET 6 D" B:2 C:8 FLAGS: - - - -
			case 0xCBF3: OutputDisassembledInstruction("SET 6 E", pc, opcode, 2); break; // "SET 6 E" B:2 C:8 FLAGS: - - - -
			case 0xCBF4: OutputDisassembledInstruction("SET 6 H", pc, opcode, 2); break; // "SET 6 H" B:2 C:8 FLAGS: - - - -
			case 0xCBF5: OutputDisassembledInstruction("SET 6 L", pc, opcode, 2); break; // "SET 6 L" B:2 C:8 FLAGS: - - - -
			case 0xCBF6: OutputDisassembledInstruction("SET 6 [HL]", pc, opcode, 2); break; // "SET 6 [HL]" B:2 C:16 FLAGS: - - - -
			case 0xCBF7: OutputDisassembledInstruction("SET 6 A", pc, opcode, 2); break; // "SET 6 A" B:2 C:8 FLAGS: - - - -
			case 0xCBF8: OutputDisassembledInstruction("SET 7 B", pc, opcode, 2); break; // "SET 7 B" B:2 C:8 FLAGS: - - - -
			case 0xCBF9: OutputDisassembledInstruction("SET 7 C", pc, opcode, 2); break; // "SET 7 C" B:2 C:8 FLAGS: - - - -
			case 0xCBFA: OutputDisassembledInstruction("SET 7 D", pc, opcode, 2); break; // "SET 7 D" B:2 C:8 FLAGS: - - - -
			case 0xCBFB: OutputDisassembledInstruction("SET 7 E", pc, opcode, 2); break; // "SET 7 E" B:2 C:8 FLAGS: - - - -
			case 0xCBFC: OutputDisassembledInstruction("SET 7 H", pc, opcode, 2); break; // "SET 7 H" B:2 C:8 FLAGS: - - - -
			case 0xCBFD: OutputDisassembledInstruction("SET 7 L", pc, opcode, 2); break; // "SET 7 L" B:2 C:8 FLAGS: - - - -
			case 0xCBFE: OutputDisassembledInstruction("SET 7 [HL]", pc, opcode, 2); break; // "SET 7 [HL]" B:2 C:16 FLAGS: - - - -
			case 0xCBFF: OutputDisassembledInstruction("SET 7 A", pc, opcode, 2); break; // "SET 7 A" B:2 C:8 FLAGS: - - - -
		}
	}

	void Cpu::OutputDisassembledInstruction(const char* instructionName, int pc, uint8_t* opcode, int totalOpBytes)
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

		m_CurrentInstructionName = output;
		Logger::Instance().Info(Domain::CPU, output);
	}

	std::string Cpu::GetCurrentInstruction()
	{
		return m_CurrentInstructionName;
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
			m_MMU.Write(address, GenerateRandomNumber(0, 255));
		}

		// hardware registers
		m_MMU.Write(0xFF00, 0xCF);
		m_MMU.Write(0xFF01, 0x00);
		m_MMU.Write(0xFF02, 0x7E);
		m_MMU.Write(0xFF04, 0xAB);
		m_MMU.Write(0xFF05, 0x00);
		m_MMU.Write(0xFF06, 0x00);
		m_MMU.Write(0xFF07, 0xF8);
		m_MMU.Write(0xFF0F, 0xE1);
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
		m_MMU.Write(0xFF48, 0x00);
		m_MMU.Write(0xFF49, 0x00);
		m_MMU.Write(0xFF4A, 0x00);
		m_MMU.Write(0xFF4B, 0x00);
		m_MMU.Write(0xFF4D, 0xFF);
		m_MMU.Write(0xFF4F, 0xFF);
		m_MMU.Write(0xFF51, 0xFF);
		m_MMU.Write(0xFF52, 0xFF);
		m_MMU.Write(0xFF53, 0xFF);
		m_MMU.Write(0xFF54, 0xFF);
		m_MMU.Write(0xFF55, 0xFF);
		m_MMU.Write(0xFF56, 0xFF);
		m_MMU.Write(0xFF68, 0xFF);
		m_MMU.Write(0xFF69, 0xFF);
		m_MMU.Write(0xFF6A, 0xFF);
		m_MMU.Write(0xFF6B, 0xFF);
		m_MMU.Write(0xFF70, 0xFF);
		m_MMU.Write(HW_INTERRUPT_ENABLE, 0x00);
	}

	void Cpu::PushSP(uint16_t value)
	{
		m_MMU.Write(--State.SP, (value >> 8) & 0xFF);
		m_MMU.Write(--State.SP, value & 0xFF);
	}

	uint16_t Cpu::PopSP()
	{
		uint8_t firstByte = m_MMU.Read(State.SP++);
		uint8_t secondByte = m_MMU.Read(State.SP++);
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
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_inc_reg(value);
		m_MMU.Write(State.HL, value);
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
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_dec_reg(value);
		m_MMU.Write(State.HL, value);
	}

	void Cpu::Instruction_add_reg(uint8_t& reg)
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

	void Cpu::Instruction_add_hl()
	{
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_add_reg(value);
		m_MMU.Write(State.HL, value);
	}

	void Cpu::Instruction_adc_reg(uint8_t& reg)
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

	void Cpu::Instruction_adc_hl()
	{
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_adc_reg(value);
		m_MMU.Write(State.HL, value);
	}

	void Cpu::Instruction_sub_reg(uint8_t& reg)
	{
		uint8_t result = State.A - reg;

		// Update flags
		SetCPUFlag(FLAG_ZERO, result == 0);
		SetCPUFlag(FLAG_SUBTRACT, true);
		SetCPUFlag(FLAG_HALF_CARRY, (State.A & 0x0F) - (reg & 0x0F) < 0);
		SetCPUFlag(FLAG_CARRY, State.A < reg);

		State.A = result;
	}

	void Cpu::Instruction_sub_hl()
	{
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_sub_reg(value);
		m_MMU.Write(State.HL, value);
	}

	void Cpu::Instruction_sbc_reg(uint8_t& reg)
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

	void Cpu::Instruction_sbc_hl()
	{
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_sbc_reg(value);
		m_MMU.Write(State.HL, value);
	}

	void Cpu::Instruction_and_reg(uint8_t& reg)
	{
		State.A = State.A & reg;

		SetCPUFlag(FLAG_ZERO, State.A == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, true);
		SetCPUFlag(FLAG_CARRY, false);
	}

	void Cpu::Instruction_and_hl()
	{
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_and_reg(value);
		m_MMU.Write(State.HL, value);
	}

	void Cpu::Instruction_xor_reg(uint8_t& reg)
	{
		State.A = State.A ^ reg;

		SetCPUFlag(FLAG_ZERO, State.A == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
		SetCPUFlag(FLAG_CARRY, false);
	}

	void Cpu::Instruction_xor_hl()
	{
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_xor_reg(value);
		m_MMU.Write(State.HL, value);
	}

	void Cpu::Instruction_or_reg(uint8_t& reg)
	{
		State.A = State.A | reg;

		SetCPUFlag(FLAG_ZERO, State.A == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, false);
		SetCPUFlag(FLAG_CARRY, false);
	}

	void Cpu::Instruction_or_hl()
	{
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_or_reg(value);
		m_MMU.Write(State.HL, value);
	}

	void Cpu::Instruction_cp_reg(uint8_t& reg)
	{
		SetCPUFlag(FLAG_ZERO, State.A == reg);
		SetCPUFlag(FLAG_SUBTRACT, true);
		SetCPUFlag(FLAG_HALF_CARRY, (State.A & 0xF) - (reg & 0xF) < 0);
		SetCPUFlag(FLAG_CARRY, State.A < reg);
	}

	void Cpu::Instruction_cp_hl()
	{
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_cp_reg(value);
		m_MMU.Write(State.HL, value);
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
		uint32_t fullResult = State.HL + reg;
		uint16_t result = static_cast<uint16_t>(fullResult);

		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, (State.HL ^ reg ^ (fullResult & 0xFFFF)) & 0x1000);
		SetCPUFlag(FLAG_CARRY, fullResult > 0xFFFF);

		State.HL = result;
	}

	void Cpu::Instruction_add_sp_e8(uint8_t& e8)
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
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_rlc_reg(value);
		m_MMU.Write(State.HL, value);
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
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_rrc_reg(value);
		m_MMU.Write(State.HL, value);
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
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_rl_reg(value);
		m_MMU.Write(State.HL, value);
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
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_rr_reg(value);
		m_MMU.Write(State.HL, value);
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
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_sla_reg(value);
		m_MMU.Write(State.HL, value);
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
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_sra_reg(value);
		m_MMU.Write(State.HL, value);
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
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_swap_reg(value);
		m_MMU.Write(State.HL, value);
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
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_srl_reg(value);
		m_MMU.Write(State.HL, value);
	}

	void Cpu::Instruction_bit_bit_reg(uint8_t& reg, uint8_t bit)
	{
		SetCPUFlag(FLAG_ZERO, (reg & (1 << bit)) == 0);
		SetCPUFlag(FLAG_SUBTRACT, false);
		SetCPUFlag(FLAG_HALF_CARRY, true);
	}

	void Cpu::Instruction_bit_bit_hl(uint8_t bit)
	{
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_bit_bit_reg(value, bit);
		m_MMU.Write(State.HL, value);
	}

	void Cpu::Instruction_res_bit_reg(uint8_t& reg, uint8_t bit)
	{
		reg &= ~(1 << bit);
	}

	void Cpu::Instruction_res_bit_hl(uint8_t bit)
	{
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_res_bit_reg(value, bit);
		m_MMU.Write(State.HL, value);
	}

	void Cpu::Instruction_set_bit_reg(uint8_t& reg, uint8_t bit)
	{
		reg |= (1 << bit);
	}

	void Cpu::Instruction_set_bit_hl(uint8_t bit)
	{
		uint8_t value = m_MMU.Read(State.HL);
		Instruction_set_bit_reg(value, bit);
		m_MMU.Write(State.HL, value);
	}

	void Cpu::ProcessTimers() 
	{
		// increment DIV register
		uint16_t internalClock = (m_MMU.Read(HW_DIV_DIVIDER_REGISTER) << 8)
			| m_MMU.Read(HW_DIV_DIVIDER_REGISTER_LOW);

		internalClock++;

		// if the div clock rolls over then we need to copy the value of TIMA to TMA
		if (internalClock == 0xFFFF)
		{
			m_MMU.Write(HW_TMA_TIMER_MODULO, m_MMU.Read(HW_TIMA_TIMER_COUNTER));
		}

		// write updated DIV register
		m_MMU.Write(HW_DIV_DIVIDER_REGISTER, (internalClock & 0xFF00) >> 8, true);
		m_MMU.Write(HW_DIV_DIVIDER_REGISTER_LOW, internalClock & 0x00FF, true);


		// https://github.com/Hacktix/GBEDG/blob/master/timers/index.md#timer-operation
		static bool lastBit;
		bool thisBit = 0;

		// 1. A bit position of the 16 - bit counter is determined based on the lower 2 bits of the TAC register
		switch (m_MMU.Read(HW_TAC_TIMER_CONTROL) & 0x03)
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
		bool timerEnabled = m_MMU.ReadRegisterBit(HW_TAC_TIMER_CONTROL, TAC_ENABLE);

		// 3. The bit taken from the DIV counter is ANDed with the Timer Enable bit. 
		//    The result of this operation will be referred to as the "AND Result".
		thisBit &= timerEnabled;

		static int countdownToInterrupt = 0;
		if (lastBit == 1 && thisBit == 0)
		{
			// now increment the TIMA register
			uint8_t tima = m_MMU.Read(HW_TIMA_TIMER_COUNTER);
			tima++;
			m_MMU.Write(HW_TIMA_TIMER_COUNTER, tima);

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
				m_MMU.WriteRegisterBit(HW_IF_INTERRUPT_FLAG, IF_TIMER, true);
				m_MMU.Write(HW_TIMA_TIMER_COUNTER, m_MMU.Read(HW_TMA_TIMER_MODULO));
			}
		}

		lastBit = thisBit;
	}
}