
#include "Instructions.h"
#include "Cpu.h"

namespace Core
{
	uint8_t Instructions::Instr_INVALID(Cpu* cpu)
	{
		return 0;
	}

	// Misc / Control Instructions
	uint8_t Instructions::Instr_0x00_MISC_NOP(Cpu* cpu) { return 4; }
	uint8_t Instructions::Instr_0x10_MISC_STOP(Cpu* cpu) { cpu->GetMMU().ResetDIVTimer(); return 4; }
	uint8_t Instructions::Instr_0x76_MISC_HALT(Cpu* cpu) { cpu->m_IsHalted; return 4; }
	uint8_t Instructions::Instr_0xCB_MISC_PREFIX(Cpu* cpu) { return 0; } // should never get called
	uint8_t Instructions::Instr_0xF3_MISC_DI(Cpu* cpu) { cpu->GetState()->IME = false; return 4; }
	uint8_t Instructions::Instr_0xFB_MISC_IE(Cpu* cpu) { cpu->GetState()->IME = true; return 4; }

	// Jumps/Calls
	uint8_t Instructions::Instr_0x18_JPCALL_JR_e8(Cpu* cpu)
	{
		cpu->GetState()->PC += (int8_t)cpu->CurrentInstruction.lowByte;

		return 12;
	}

	uint8_t Instructions::Instr_0x20_JPCALL_JR_NZ_e8(Cpu* cpu)
	{
		// note: "e8" in the description refers to a signed char
		int8_t offset = cpu->CurrentInstruction.lowByte;
		if (cpu->GetCPUFlag(FLAG_ZERO) == 0)
		{
			cpu->GetState()->PC += offset;
			return 12;
		}
		else
		{
			//cpu->GetState()->PC++;
			return 8;
		}
	}

	uint8_t Instructions::Instr_0x28_JPCALL_JR_Z_e8(Cpu* cpu)
	{
		// note: "e8" in the description refers to a signed char
		int8_t offset = cpu->CurrentInstruction.lowByte;
		if (cpu->GetCPUFlag(FLAG_ZERO) == 1)
		{
			cpu->GetState()->PC += offset;
			return 12;
		}
		else
		{
			return 8;
		}
	}

	uint8_t Instructions::Instr_0x30_JPCALL_JR_NC_e8(Cpu* cpu)
	{
		// note: "s8" in the description refers to a signed char
		int8_t offset = cpu->CurrentInstruction.lowByte;
		if (!cpu->GetCPUFlag(FLAG_CARRY))
		{
			cpu->GetState()->PC += offset;
			return 12;
		}
		else
		{
			return 8;
		}	
	}

	uint8_t Instructions::Instr_0x38_JPCALL_JR_C_e8(Cpu* cpu)
	{
		// note: "s8" in the description refers to a signed char
		int8_t offset = cpu->CurrentInstruction.lowByte;
		if (cpu->GetCPUFlag(FLAG_CARRY))
		{
			cpu->GetState()->PC += offset;
			return 12;
		}
		else
		{
			return 8;
		}
	}

	uint8_t Instructions::Instr_0xC0_JPCALL_RET_NZ(Cpu* cpu)
	{
		if (!cpu->GetCPUFlag(FLAG_ZERO))
		{
			cpu->GetState()->PC = cpu->PopSP();
			return 20;
		}

		return 8;
	}

	uint8_t Instructions::Instr_0xC2_JPCALL_JP_NZ_a16(Cpu* cpu)
	{
		if (!cpu->GetCPUFlag(FLAG_ZERO))
		{
			uint16_t offset = (cpu->CurrentInstruction.highByte << 8) | (cpu->CurrentInstruction.lowByte);
			cpu->GetState()->PC = offset;

			return 16;
		}

		return 12;
	}

	uint8_t Instructions::Instr_0xC3_JPCALL_JP_a16(Cpu* cpu)
	{
		uint16_t offset = (cpu->CurrentInstruction.highByte << 8) | (cpu->CurrentInstruction.lowByte);
		cpu->GetState()->PC = offset;

		return 16;
	}

	uint8_t Instructions::Instr_0xC4_JPCALL_CALL_NZ_a16(Cpu* cpu)
	{
		if (!cpu->GetCPUFlag(FLAG_ZERO))
		{
			cpu->PushSP(cpu->GetState()->PC);
			cpu->GetState()->PC = (cpu->CurrentInstruction.highByte << 8) | (cpu->CurrentInstruction.lowByte);

			return 24;
		}

		return 12;
	}

	uint8_t Instructions::Instr_0xC7_JPCALL_RST_00(Cpu* cpu)
	{
		cpu->PushSP(cpu->GetState()->PC);
		cpu->GetState()->PC = 0x00;

		return 16;
	}

	uint8_t Instructions::Instr_0xC8_JPCALL_RET_Z(Cpu* cpu)
	{
		if (cpu->GetCPUFlag(FLAG_ZERO))
		{
			cpu->GetState()->PC = cpu->PopSP();
			return 20;
		}

		return 8;
	}

	uint8_t Instructions::Instr_0xC9_JPCALL_RET(Cpu* cpu)
	{
		cpu->GetState()->PC = cpu->PopSP();

		return 16;
	}

	uint8_t Instructions::Instr_0xCA_JPCALL_JP_Z_a16(Cpu* cpu)
	{
		if (cpu->GetCPUFlag(FLAG_ZERO))
		{
			uint16_t offset = (cpu->CurrentInstruction.highByte << 8) | (cpu->CurrentInstruction.lowByte);
			cpu->GetState()->PC = offset;

			return 16;
		}

		return 12;
	}

	uint8_t Instructions::Instr_0xCC_JPCALL_CALL_Z_a16(Cpu* cpu)
	{
		if (cpu->GetCPUFlag(FLAG_ZERO))
		{
			cpu->PushSP(cpu->GetState()->PC);
			cpu->GetState()->PC = (cpu->CurrentInstruction.highByte << 8) | (cpu->CurrentInstruction.lowByte);
			return 24;
		}

		return 12;
	}

	uint8_t Instructions::Instr_0xCD_JPCALL_CALL_a16(Cpu* cpu)
	{
		cpu->PushSP(cpu->GetState()->PC);
		cpu->GetState()->PC = (cpu->CurrentInstruction.highByte << 8) | (cpu->CurrentInstruction.lowByte);

		return 24;
	}

	uint8_t Instructions::Instr_0xCF_JPCALL_RST_08(Cpu* cpu)
	{
		cpu->PushSP(cpu->GetState()->PC);
		cpu->GetState()->PC = 0x08;

		return 16;
	}

	uint8_t Instructions::Instr_0xD0_JPCALL_RET_NC(Cpu* cpu)
	{
		if (!cpu->GetCPUFlag(FLAG_CARRY))
		{
			cpu->GetState()->PC = cpu->PopSP();
			return 20;
		}

		return 8;
	}

	uint8_t Instructions::Instr_0xD2_JPCALL_JP_NC_a16(Cpu* cpu)
	{
		if (!cpu->GetCPUFlag(FLAG_CARRY))
		{
			uint16_t offset = (cpu->CurrentInstruction.highByte << 8) | (cpu->CurrentInstruction.lowByte);
			cpu->GetState()->PC = offset;

			return 16;
		}

		return 12;
	}

	uint8_t Instructions::Instr_0xD4_JPCALL_VALL_NC_a16(Cpu* cpu)
	{
		if (!cpu->GetCPUFlag(FLAG_CARRY))
		{
			cpu->PushSP(cpu->GetState()->PC);
			cpu->GetState()->PC = (cpu->CurrentInstruction.highByte << 8) | (cpu->CurrentInstruction.lowByte);

			return 24;
		}

		return 12;
	}

	uint8_t Instructions::Instr_0xD7_JPCALL_RST_10(Cpu* cpu)
	{
		cpu->PushSP(cpu->GetState()->PC);
		cpu->GetState()->PC = 0x10;

		return 16;
	}

	uint8_t Instructions::Instr_0xD8_JPCALL_RET_C(Cpu* cpu)
	{
		if (cpu->GetCPUFlag(FLAG_CARRY))
		{
			cpu->GetState()->PC = cpu->PopSP();
			return 20;
		}

		return 8;
	}

	uint8_t Instructions::Instr_0xD9_JPCALL_RETI(Cpu* cpu)
	{
		cpu->GetState()->PC = cpu->PopSP();
		cpu->GetState()->IME = true;

		return 16;
	}

	uint8_t Instructions::Instr_0xDA_JPCALL_JP_C_a16(Cpu* cpu)
	{
		if (cpu->GetCPUFlag(FLAG_CARRY))
		{
			uint16_t offset = (cpu->CurrentInstruction.highByte << 8) | (cpu->CurrentInstruction.lowByte);
			cpu->GetState()->PC = offset;

			return 16;
		}

		return 12;
	}

	uint8_t Instructions::Instr_0xDC_JPCALL_CALL_C_a16(Cpu* cpu)
	{
		if (cpu->GetCPUFlag(FLAG_CARRY))
		{
			cpu->PushSP(cpu->GetState()->PC);
			cpu->GetState()->PC = (cpu->CurrentInstruction.highByte << 8) | (cpu->CurrentInstruction.lowByte);

			return 24;
		}

		return 12;
	}

	uint8_t Instructions::Instr_0xDF_JPCALL_RST_18(Cpu* cpu)
	{
		cpu->PushSP(cpu->GetState()->PC);
		cpu->GetState()->PC = 0x18;

		return 16;
	}

	uint8_t Instructions::Instr_0xE7_JPCALL_RST_20(Cpu* cpu)
	{
		cpu->PushSP(cpu->GetState()->PC);
		cpu->GetState()->PC = 0x20;

		return 16;
	}

	uint8_t Instructions::Instr_0xE9_JPCALL_JP_HL(Cpu* cpu)
	{
		cpu->GetState()->PC = cpu->GetState()->HL;

		return 4;
	}

	uint8_t Instructions::Instr_0xEF_JPCALL_RST_28(Cpu* cpu)
	{
		cpu->PushSP(cpu->GetState()->PC);
		cpu->GetState()->PC = 0x28;

		return 16;
	}

	uint8_t Instructions::Instr_0xF7_JPCALL_RST_30(Cpu* cpu)
	{
		cpu->PushSP(cpu->GetState()->PC);
		cpu->GetState()->PC = 0x30;

		return 16;
	}

	uint8_t Instructions::Instr_0xFF_JPCALL_RST_38(Cpu* cpu)
	{
		cpu->PushSP(cpu->GetState()->PC);
		cpu->GetState()->PC = 0x38;

		return 16;	
	}


	// 8-bit Load Instructions
	uint8_t Instructions::Instructions::Instr_0x02_8LD_BC_A(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		cpu->GetMMU().Write(state->BC, state->A);
		return 8;
	}

	uint8_t Instructions::Instructions::Instr_0x12_8LD_DE_A(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		cpu->GetMMU().Write(state->DE, state->A);
		return 8;
	}

	uint8_t Instructions::Instr_0x0A_8LD_A_BC(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->A = cpu->GetMMU().Read(state->BC);
		return 8;
	}

	uint8_t Instructions::Instr_0x1A_8LD_A_DE(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->A = cpu->GetMMU().Read(state->DE);
		return 8;
	}

	uint8_t Instructions::Instr_0x46_8LD_B_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->B = cpu->GetMMU().Read(state->HL);
		return 8;
	}

	uint8_t Instructions::Instr_0x4E_8LD_C_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->C = cpu->GetMMU().Read(state->HL);
		return 8;
	}

	uint8_t Instructions::Instr_0x56_8LD_D_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->D = cpu->GetMMU().Read(state->HL);
		return 8;
	}

	uint8_t Instructions::Instr_0x5E_8LD_E_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->E = cpu->GetMMU().Read(state->HL);
		return 8;
	}

	uint8_t Instructions::Instr_0x66_8LD_H_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->H = cpu->GetMMU().Read(state->HL);
		return 8;
	}

	uint8_t Instructions::Instr_0x6E_8LD_L_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->L = cpu->GetMMU().Read(state->HL);
		return 8;
	}

	uint8_t Instructions::Instr_0x7E_8LD_A_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->A = cpu->GetMMU().Read(state->HL);
		return 8;
	}

	uint8_t Instructions::Instr_0x06_8LD_B_e8(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->B = cpu->CurrentInstruction.lowByte;
		return 8;
	}

	uint8_t Instructions::Instr_0x0E_8LD_C_e8(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->C = cpu->CurrentInstruction.lowByte;
		return 8;
	}

	uint8_t Instructions::Instr_0x16_8LD_D_e8(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->D = cpu->CurrentInstruction.lowByte;
		return 8;
	}

	uint8_t Instructions::Instr_0x1E_8LD_E_e8(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->E = cpu->CurrentInstruction.lowByte;
		return 8;
	}

	uint8_t Instructions::Instr_0x26_8LD_H_e8(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->H = cpu->CurrentInstruction.lowByte;
		return 8;
	}

	uint8_t Instructions::Instr_0x2E_8LD_L_e8(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->L = cpu->CurrentInstruction.lowByte;
		return 8;
	}

	uint8_t Instructions::Instr_0x3A_8LD_A_HLDEC(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->A = cpu->GetMMU().Read(state->HL);
		state->HL--;

		return  8;
	}

	uint8_t Instructions::Instr_0x22_8LD_HLINC_A(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		cpu->GetMMU().Write(state->HL, state->A);
		state->HL++;

		return  8;
	}

	uint8_t Instructions::Instr_0x2A_8LD_A_HLINC(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->A = cpu->GetMMU().Read(state->HL);
		state->HL++;

		return  8;
	}

	uint8_t Instructions::Instr_0x32_8LD_HLDEC_A(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		cpu->GetMMU().Write(state->HL, state->A);
		state->HL--;

		return  8;
	}

	uint8_t Instructions::Instr_0x36_8LD_HL_e8(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		cpu->GetMMU().Write(state->HL, cpu->CurrentInstruction.lowByte);

		return  12;
	}

	uint8_t Instructions::Instr_0x3E_8LD_A_e8(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->A = cpu->CurrentInstruction.lowByte;

		return  8;
	}

	uint8_t Instructions::Instr_0x40_8LD_B_B(Cpu* cpu) { cpu->GetState()->B = cpu->GetState()->B;return 4; }
	uint8_t Instructions::Instr_0x41_8LD_B_C(Cpu* cpu) { cpu->GetState()->B = cpu->GetState()->C;return 4; }
	uint8_t Instructions::Instr_0x42_8LD_B_D(Cpu* cpu) { cpu->GetState()->B = cpu->GetState()->D;return 4; }
	uint8_t Instructions::Instr_0x43_8LD_B_E(Cpu* cpu) { cpu->GetState()->B = cpu->GetState()->E;return 4; }
	uint8_t Instructions::Instr_0x44_8LD_B_H(Cpu* cpu) { cpu->GetState()->B = cpu->GetState()->H;return 4; }
	uint8_t Instructions::Instr_0x45_8LD_B_L(Cpu* cpu) { cpu->GetState()->B = cpu->GetState()->L;return 4; }
	uint8_t Instructions::Instr_0x47_8LD_B_A(Cpu* cpu) { cpu->GetState()->B = cpu->GetState()->A;return 4; }
	uint8_t Instructions::Instr_0x48_8LD_C_B(Cpu* cpu) { cpu->GetState()->C = cpu->GetState()->B;return 4; }
	uint8_t Instructions::Instr_0x49_8LD_C_C(Cpu* cpu) { cpu->GetState()->C = cpu->GetState()->C;return 4; }
	uint8_t Instructions::Instr_0x4A_8LD_C_D(Cpu* cpu) { cpu->GetState()->C = cpu->GetState()->D;return 4; }
	uint8_t Instructions::Instr_0x4B_8LD_C_E(Cpu* cpu) { cpu->GetState()->C = cpu->GetState()->E;return 4; }
	uint8_t Instructions::Instr_0x4C_8LD_C_H(Cpu* cpu) { cpu->GetState()->C = cpu->GetState()->H;return 4; }
	uint8_t Instructions::Instr_0x4D_8LD_C_L(Cpu* cpu) { cpu->GetState()->C = cpu->GetState()->L;return 4; }
	uint8_t Instructions::Instr_0x4F_8LD_C_A(Cpu* cpu) { cpu->GetState()->C = cpu->GetState()->A;return 4; }
	uint8_t Instructions::Instr_0x50_8LD_D_B(Cpu* cpu) { cpu->GetState()->D = cpu->GetState()->B;return 4; }
	uint8_t Instructions::Instr_0x51_8LD_D_C(Cpu* cpu) { cpu->GetState()->D = cpu->GetState()->C;return 4; }
	uint8_t Instructions::Instr_0x52_8LD_D_D(Cpu* cpu) { cpu->GetState()->D = cpu->GetState()->D;return 4; }
	uint8_t Instructions::Instr_0x53_8LD_D_E(Cpu* cpu) { cpu->GetState()->D = cpu->GetState()->E;return 4; }
	uint8_t Instructions::Instr_0x54_8LD_D_H(Cpu* cpu) { cpu->GetState()->D = cpu->GetState()->H;return 4; }
	uint8_t Instructions::Instr_0x55_8LD_D_L(Cpu* cpu) { cpu->GetState()->D = cpu->GetState()->L;return 4; }
	uint8_t Instructions::Instr_0x57_8LD_D_A(Cpu* cpu) { cpu->GetState()->D = cpu->GetState()->A;return 4; }
	uint8_t Instructions::Instr_0x58_8LD_E_B(Cpu* cpu) { cpu->GetState()->E = cpu->GetState()->B;return 4; }
	uint8_t Instructions::Instr_0x59_8LD_E_C(Cpu* cpu) { cpu->GetState()->E = cpu->GetState()->C;return 4; }
	uint8_t Instructions::Instr_0x5A_8LD_E_D(Cpu* cpu) { cpu->GetState()->E = cpu->GetState()->D;return 4; }
	uint8_t Instructions::Instr_0x5B_8LD_E_E(Cpu* cpu) { cpu->GetState()->E = cpu->GetState()->E;return 4; }
	uint8_t Instructions::Instr_0x5C_8LD_E_H(Cpu* cpu) { cpu->GetState()->E = cpu->GetState()->H;return 4; }
	uint8_t Instructions::Instr_0x5D_8LD_E_L(Cpu* cpu) { cpu->GetState()->E = cpu->GetState()->L;return 4; }
	uint8_t Instructions::Instr_0x5F_8LD_E_A(Cpu* cpu) { cpu->GetState()->E = cpu->GetState()->A;return 4; }
	uint8_t Instructions::Instr_0x60_8LD_H_B(Cpu* cpu) { cpu->GetState()->H = cpu->GetState()->B;return 4; }
	uint8_t Instructions::Instr_0x61_8LD_H_C(Cpu* cpu) { cpu->GetState()->H = cpu->GetState()->C;return 4; }
	uint8_t Instructions::Instr_0x62_8LD_H_D(Cpu* cpu) { cpu->GetState()->H = cpu->GetState()->D;return 4; }
	uint8_t Instructions::Instr_0x63_8LD_H_E(Cpu* cpu) { cpu->GetState()->H = cpu->GetState()->E;return 4; }
	uint8_t Instructions::Instr_0x64_8LD_H_H(Cpu* cpu) { cpu->GetState()->H = cpu->GetState()->H;return 4; }
	uint8_t Instructions::Instr_0x65_8LD_H_L(Cpu* cpu) { cpu->GetState()->H = cpu->GetState()->L;return 4; }
	uint8_t Instructions::Instr_0x67_8LD_H_A(Cpu* cpu) { cpu->GetState()->H = cpu->GetState()->A;return 4; }
	uint8_t Instructions::Instr_0x68_8LD_L_B(Cpu* cpu) { cpu->GetState()->L = cpu->GetState()->B;return 4; }
	uint8_t Instructions::Instr_0x69_8LD_L_C(Cpu* cpu) { cpu->GetState()->L = cpu->GetState()->C;return 4; }
	uint8_t Instructions::Instr_0x6A_8LD_L_D(Cpu* cpu) { cpu->GetState()->L = cpu->GetState()->D;return 4; }
	uint8_t Instructions::Instr_0x6B_8LD_L_E(Cpu* cpu) { cpu->GetState()->L = cpu->GetState()->E;return 4; }
	uint8_t Instructions::Instr_0x6C_8LD_L_H(Cpu* cpu) { cpu->GetState()->L = cpu->GetState()->H;return 4; }
	uint8_t Instructions::Instr_0x6D_8LD_L_L(Cpu* cpu) { cpu->GetState()->L = cpu->GetState()->L;return 4; }
	uint8_t Instructions::Instr_0x6F_8LD_L_A(Cpu* cpu) { cpu->GetState()->L = cpu->GetState()->A;return 4; }
	uint8_t Instructions::Instr_0x78_8LD_A_B(Cpu* cpu) { cpu->GetState()->A = cpu->GetState()->B;return 4; }
	uint8_t Instructions::Instr_0x79_8LD_A_C(Cpu* cpu) { cpu->GetState()->A = cpu->GetState()->C;return 4; }
	uint8_t Instructions::Instr_0x7A_8LD_A_D(Cpu* cpu) { cpu->GetState()->A = cpu->GetState()->D;return 4; }
	uint8_t Instructions::Instr_0x7B_8LD_A_E(Cpu* cpu) { cpu->GetState()->A = cpu->GetState()->E;return 4; }
	uint8_t Instructions::Instr_0x7C_8LD_A_H(Cpu* cpu) { cpu->GetState()->A = cpu->GetState()->H;return 4; }
	uint8_t Instructions::Instr_0x7D_8LD_A_L(Cpu* cpu) { cpu->GetState()->A = cpu->GetState()->L;return 4; }
	uint8_t Instructions::Instr_0x7F_8LD_A_A(Cpu* cpu) { cpu->GetState()->A = cpu->GetState()->A;return 4; }
	uint8_t Instructions::Instr_0x70_8LD_HL_B(Cpu* cpu) { cpu->GetMMU().Write(cpu->GetState()->HL, cpu->GetState()->B); return 8; }
	uint8_t Instructions::Instr_0x71_8LD_HL_C(Cpu* cpu) { cpu->GetMMU().Write(cpu->GetState()->HL, cpu->GetState()->C); return 8; }
	uint8_t Instructions::Instr_0x72_8LD_HL_D(Cpu* cpu) { cpu->GetMMU().Write(cpu->GetState()->HL, cpu->GetState()->D); return 8; }
	uint8_t Instructions::Instr_0x73_8LD_HL_E(Cpu* cpu) { cpu->GetMMU().Write(cpu->GetState()->HL, cpu->GetState()->E); return 8; }
	uint8_t Instructions::Instr_0x74_8LD_HL_H(Cpu* cpu) { cpu->GetMMU().Write(cpu->GetState()->HL, cpu->GetState()->H); return 8; }
	uint8_t Instructions::Instr_0x75_8LD_HL_L(Cpu* cpu) { cpu->GetMMU().Write(cpu->GetState()->HL, cpu->GetState()->L); return 8; }
	uint8_t Instructions::Instr_0x77_8LD_HL_A(Cpu* cpu) { cpu->GetMMU().Write(cpu->GetState()->HL, cpu->GetState()->A); return 8; }
	uint8_t Instructions::Instr_0xE0_8LD_a8_A(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		uint8_t offset = cpu->CurrentInstruction.lowByte;
		cpu->GetMMU().Write(0xFF00 + offset, state->A);

		return 12;
	}

	uint8_t Instructions::Instr_0xE2_8LD_CADDR_A(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		cpu->GetMMU().Write(0xFF00 + state->C, state->A);

		return 8;
	}

	uint8_t Instructions::Instr_0xEA_8LD_a16_A(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		cpu->GetMMU().Write((cpu->CurrentInstruction.highByte << 8) | (cpu->CurrentInstruction.lowByte), state->A);

		return 16;
	}

	uint8_t Instructions::Instr_0xF0_8LD_A_a8(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->A = cpu->GetMMU().Read(0xFF00 + cpu->CurrentInstruction.lowByte);

		return 12;
	}

	uint8_t Instructions::Instr_0xF2_8LD_A_CADDR(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->A = cpu->GetMMU().Read(0xFF00 + state->C);

		return 8;
	}

	uint8_t Instructions::Instr_0xFA_8LD_A_a16(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->A = cpu->GetMMU().Read((cpu->CurrentInstruction.highByte << 8) | (cpu->CurrentInstruction.lowByte));

		return 16;
	}


	// 16-bit Load Instructions
	uint8_t Instructions::Instr_0x01_16LD_BC_e16(Cpu* cpu) { cpu->GetState()->BC = (cpu->CurrentInstruction.highByte << 8) | (cpu->CurrentInstruction.lowByte); return 12; }
	uint8_t Instructions::Instr_0x11_16LD_DE_e16(Cpu* cpu) { cpu->GetState()->DE = (cpu->CurrentInstruction.highByte << 8) | (cpu->CurrentInstruction.lowByte); return 12; }
	uint8_t Instructions::Instr_0x21_16LD_HL_e16(Cpu* cpu) { cpu->GetState()->HL = (cpu->CurrentInstruction.highByte << 8) | (cpu->CurrentInstruction.lowByte); return 12; }
	uint8_t Instructions::Instr_0x31_16LD_SP_e16(Cpu* cpu) { cpu->GetState()->SP = (cpu->CurrentInstruction.highByte << 8) | (cpu->CurrentInstruction.lowByte); return 12; }
	
	uint8_t Instructions::Instr_0x08_16LD_a16_SP(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint16_t addr = (cpu->CurrentInstruction.highByte << 8) | (cpu->CurrentInstruction.lowByte);
		cpu->GetMMU().Write(addr, state->SP & 0x00FF);
		cpu->GetMMU().Write(addr + 1, (state->SP & 0xFF00) >> 8);

		return 20;
	}

	uint8_t Instructions::Instr_0xC1_16LD_POP_BC(Cpu* cpu)
	{
		cpu->GetState()->BC = cpu->PopSP();

		return 12;
	}

	uint8_t Instructions::Instr_0xC5_16LD_PUSH_BC(Cpu* cpu)
	{
		cpu->PushSP(cpu->GetState()->BC);

		return 16;
	}

	uint8_t Instructions::Instr_0xD1_16LD_POP_DE(Cpu* cpu)
	{
		cpu->GetState()->DE = cpu->PopSP();

		return 12;
	}

	uint8_t Instructions::Instr_0xD5_16LD_PUSH_DE(Cpu* cpu)
	{
		cpu->PushSP(cpu->GetState()->DE);

		return 16;
	}

	uint8_t Instructions::Instr_0xE1_16LD_POP_HL(Cpu* cpu)
	{
		cpu->GetState()->HL = cpu->PopSP();

		return 12;
	}

	uint8_t Instructions::Instr_0xE5_16LD_PUSH_DL(Cpu* cpu)
	{
		cpu->PushSP(cpu->GetState()->HL);

		return 16;
	}

	uint8_t Instructions::Instr_0xF1_16LD_POP_AF(Cpu* cpu)
	{
		cpu->GetState()->AF = (cpu->PopSP() & 0xFFF0);

		return 12;
	}

	uint8_t Instructions::Instr_0xF5_16LD_PUSH_AF(Cpu* cpu)
	{
		cpu-> PushSP(cpu->GetState()->AF);

		return 16;
	}

	uint8_t Instructions::Instr_0xF8_16LD_SP_e8(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		int8_t signedValue = static_cast<int8_t>(cpu->CurrentInstruction.lowByte);
		int32_t fullResult = state->SP + signedValue;
		uint16_t result = static_cast<uint16_t>(fullResult);

		cpu->SetCPUFlag(FLAG_ZERO, false);
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, ((state->SP ^ signedValue ^ (fullResult & 0xFFFF)) & 0x10) == 0x10);
		cpu->SetCPUFlag(FLAG_CARRY, ((state->SP ^ signedValue ^ (fullResult & 0xFFFF)) & 0x100) == 0x100);

		state->HL = result;

		return 12;
	}

	uint8_t Instructions::Instr_0xF9_16LD_SP_HL(Cpu* cpu)
	{
		cpu->GetState()->SP = cpu->GetState()->HL;

		return 8;
	}


	// 8-bit Arithmetic/Logical Instructions
	uint8_t Instructions::Instr_0x04_8ALU_INC_B(Cpu* cpu)  { INC(cpu, cpu->GetState()->B); return 4; }
	uint8_t Instructions::Instr_0x0C_8ALU_INC_C(Cpu* cpu)  { INC(cpu, cpu->GetState()->C); return 4; }
	uint8_t Instructions::Instr_0x14_8ALU_INC_D(Cpu* cpu)  { INC(cpu, cpu->GetState()->D); return 4; }
	uint8_t Instructions::Instr_0x1C_8ALU_INC_E(Cpu* cpu)  { INC(cpu, cpu->GetState()->E); return 4; }
	uint8_t Instructions::Instr_0x24_8ALU_INC_H(Cpu* cpu)  { INC(cpu, cpu->GetState()->H); return 4; }
	uint8_t Instructions::Instr_0x2C_8ALU_INC_L(Cpu* cpu)  { INC(cpu, cpu->GetState()->L); return 4; }
	uint8_t Instructions::Instr_0x34_8ALU_INC_HL(Cpu* cpu) { INC_HL(cpu); return 12; }
	uint8_t Instructions::Instr_0x3C_8ALU_INC_A(Cpu* cpu)  { INC(cpu, cpu->GetState()->A); return 4; }
	uint8_t Instructions::Instr_0x05_8ALU_DEC_B(Cpu* cpu)  { DEC(cpu, cpu->GetState()->B); return 4; }
	uint8_t Instructions::Instr_0x0D_8ALU_DEC_C(Cpu* cpu)  { DEC(cpu, cpu->GetState()->C); return 4; }
	uint8_t Instructions::Instr_0x15_8ALU_DEC_D(Cpu* cpu)  { DEC(cpu, cpu->GetState()->D); return 4; }
	uint8_t Instructions::Instr_0x1D_8ALU_DEC_E(Cpu* cpu)  { DEC(cpu, cpu->GetState()->E); return 4; }
	uint8_t Instructions::Instr_0x25_8ALU_DEC_H(Cpu* cpu)  { DEC(cpu, cpu->GetState()->H); return 4; }
	uint8_t Instructions::Instr_0x2D_8ALU_DEC_L(Cpu* cpu)  { DEC(cpu, cpu->GetState()->L); return 4; }
	uint8_t Instructions::Instr_0x35_8ALU_DEC_HL(Cpu* cpu) { DEC_HL(cpu); return 12; }
	uint8_t Instructions::Instr_0x3D_8ALU_DEC_A(Cpu* cpu)  { DEC(cpu, cpu->GetState()->A); return 4; }
	uint8_t Instructions::Instr_0x80_8ALU_ADD_B(Cpu* cpu)  { ADD(cpu, cpu->GetState()->B); return 4; }
	uint8_t Instructions::Instr_0x81_8ALU_ADD_C(Cpu* cpu)  { ADD(cpu, cpu->GetState()->C); return 4; }
	uint8_t Instructions::Instr_0x82_8ALU_ADD_D(Cpu* cpu)  { ADD(cpu, cpu->GetState()->D); return 4; }
	uint8_t Instructions::Instr_0x83_8ALU_ADD_E(Cpu* cpu)  { ADD(cpu, cpu->GetState()->E); return 4; }
	uint8_t Instructions::Instr_0x84_8ALU_ADD_H(Cpu* cpu)  { ADD(cpu, cpu->GetState()->H); return 4; }
	uint8_t Instructions::Instr_0x85_8ALU_ADD_L(Cpu* cpu)  { ADD(cpu, cpu->GetState()->L); return 4; }
	uint8_t Instructions::Instr_0x86_8ALU_ADD_HL(Cpu* cpu) { ADD_HL(cpu); return 8; }
	uint8_t Instructions::Instr_0x87_8ALU_ADD_A(Cpu* cpu)  { ADD(cpu, cpu->GetState()->A); return 4; }
	uint8_t Instructions::Instr_0xC6_8ALU_ADD_e8(Cpu* cpu) { ADD(cpu, cpu->CurrentInstruction.lowByte); return 8; }
	uint8_t Instructions::Instr_0x88_8ALU_ADC_B(Cpu* cpu)  { ADC(cpu, cpu->GetState()->B); return 4; }
	uint8_t Instructions::Instr_0x89_8ALU_ADC_C(Cpu* cpu)  { ADC(cpu, cpu->GetState()->C); return 4; }
	uint8_t Instructions::Instr_0x8A_8ALU_ADC_D(Cpu* cpu)  { ADC(cpu, cpu->GetState()->D); return 4; }
	uint8_t Instructions::Instr_0x8B_8ALU_ADC_E(Cpu* cpu)  { ADC(cpu, cpu->GetState()->E); return 4; }
	uint8_t Instructions::Instr_0x8C_8ALU_ADC_H(Cpu* cpu)  { ADC(cpu, cpu->GetState()->H); return 4; }
	uint8_t Instructions::Instr_0x8D_8ALU_ADC_L(Cpu* cpu)  { ADC(cpu, cpu->GetState()->L); return 4; }
	uint8_t Instructions::Instr_0x8E_8ALU_ADC_HL(Cpu* cpu) { ADC_HL(cpu); return 8; }
	uint8_t Instructions::Instr_0x8F_8ALU_ADC_A(Cpu* cpu)  { ADC(cpu, cpu->GetState()->A); return 4; }
	uint8_t Instructions::Instr_0xCE_8ALU_ADC_e8(Cpu* cpu) { ADC(cpu, cpu->CurrentInstruction.lowByte); return 8; }
	uint8_t Instructions::Instr_0x90_8ALU_SUB_B(Cpu* cpu)  { SUB(cpu, cpu->GetState()->B); return 4; }
	uint8_t Instructions::Instr_0x91_8ALU_SUB_C(Cpu* cpu)  { SUB(cpu, cpu->GetState()->C); return 4; }
	uint8_t Instructions::Instr_0x92_8ALU_SUB_D(Cpu* cpu)  { SUB(cpu, cpu->GetState()->D); return 4; }
	uint8_t Instructions::Instr_0x93_8ALU_SUB_E(Cpu* cpu)  { SUB(cpu, cpu->GetState()->E); return 4; }
	uint8_t Instructions::Instr_0x94_8ALU_SUB_H(Cpu* cpu)  { SUB(cpu, cpu->GetState()->H); return 4; }
	uint8_t Instructions::Instr_0x95_8ALU_SUB_L(Cpu* cpu)  { SUB(cpu, cpu->GetState()->L); return 4; }
	uint8_t Instructions::Instr_0x96_8ALU_SUB_HL(Cpu* cpu) { SUB_HL(cpu); return 8; }

	uint8_t Instructions::Instr_0x97_8ALU_SUB_A(Cpu* cpu)
	{
		// SUB A A is a special case where the flags need to be 1 1 0 0
		SUB(cpu, cpu->GetState()->A);
		cpu->SetCPUFlag(FLAG_ZERO, true);
		cpu->SetCPUFlag(FLAG_SUBTRACT, true);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);
		cpu->SetCPUFlag(FLAG_CARRY, false);
		
		return 4;
	}

	uint8_t Instructions::Instr_0xD6_8ALU_SUB_e8(Cpu* cpu) { SUB(cpu, cpu->CurrentInstruction.lowByte); return 4; }
	uint8_t Instructions::Instr_0x98_8ALU_SBC_B(Cpu* cpu)  { SBC(cpu, cpu->GetState()->B); return 4; }
	uint8_t Instructions::Instr_0x99_8ALU_SBC_D(Cpu* cpu)  { SBC(cpu, cpu->GetState()->C); return 4; }
	uint8_t Instructions::Instr_0x9A_8ALU_SBC_D(Cpu* cpu)  { SBC(cpu, cpu->GetState()->D); return 4; }
	uint8_t Instructions::Instr_0x9B_8ALU_SBC_E(Cpu* cpu)  { SBC(cpu, cpu->GetState()->E); return 4; }
	uint8_t Instructions::Instr_0x9C_8ALU_SBC_H(Cpu* cpu)  { SBC(cpu, cpu->GetState()->H); return 4; }
	uint8_t Instructions::Instr_0x9D_8ALU_SBC_L(Cpu* cpu)  { SBC(cpu, cpu->GetState()->L); return 4; }
	uint8_t Instructions::Instr_0x9E_8ALU_SBC_HL(Cpu* cpu) { SBC_HL(cpu); return 8; }
	uint8_t Instructions::Instr_0x9F_8ALU_SBC_A(Cpu* cpu)  { SBC(cpu, cpu->GetState()->A); return 4; }
	uint8_t Instructions::Instr_0xDE_8ALU_SBC_e8(Cpu* cpu) { SBC(cpu, cpu->CurrentInstruction.lowByte); return 8; }
	uint8_t Instructions::Instr_0xA0_8ALU_AND_B(Cpu* cpu)  { AND(cpu, cpu->GetState()->B); return 4; }
	uint8_t Instructions::Instr_0xA1_8ALU_AND_C(Cpu* cpu)  { AND(cpu, cpu->GetState()->C); return 4; }
	uint8_t Instructions::Instr_0xA2_8ALU_AND_D(Cpu* cpu)  { AND(cpu, cpu->GetState()->D); return 4; }
	uint8_t Instructions::Instr_0xA3_8ALU_AND_E(Cpu* cpu)  { AND(cpu, cpu->GetState()->E); return 4; }
	uint8_t Instructions::Instr_0xA4_8ALU_AND_H(Cpu* cpu)  { AND(cpu, cpu->GetState()->H); return 4; }
	uint8_t Instructions::Instr_0xA5_8ALU_AND_L(Cpu* cpu)  { AND(cpu, cpu->GetState()->L); return 4; }
	uint8_t Instructions::Instr_0xA6_8ALU_AND_HL(Cpu* cpu) { AND_HL(cpu); return 8; }
	uint8_t Instructions::Instr_0xA7_8ALU_AND_A(Cpu* cpu)  { AND(cpu, cpu->GetState()->A); return 4; }
	uint8_t Instructions::Instr_0xE6_8ALU_AND_e8(Cpu* cpu) { AND(cpu, cpu->CurrentInstruction.lowByte); return 8; }
	uint8_t Instructions::Instr_0xA8_8ALU_XOR_B(Cpu* cpu)  { XOR(cpu, cpu->GetState()->B); return 4; }
	uint8_t Instructions::Instr_0xA9_8ALU_XOR_C(Cpu* cpu)  { XOR(cpu, cpu->GetState()->C); return 4; }
	uint8_t Instructions::Instr_0xAA_8ALU_XOR_D(Cpu* cpu)  { XOR(cpu, cpu->GetState()->D); return 4; }
	uint8_t Instructions::Instr_0xAB_8ALU_XOR_E(Cpu* cpu)  { XOR(cpu, cpu->GetState()->E); return 4; }
	uint8_t Instructions::Instr_0xAC_8ALU_XOR_H(Cpu* cpu)  { XOR(cpu, cpu->GetState()->H); return 4; }
	uint8_t Instructions::Instr_0xAD_8ALU_XOR_L(Cpu* cpu)  { XOR(cpu, cpu->GetState()->L); return 4; }
	uint8_t Instructions::Instr_0xAE_8ALU_XOR_HL(Cpu* cpu) { XOR_HL(cpu); return 8; }
	uint8_t Instructions::Instr_0xAF_8ALU_XOR_A(Cpu* cpu)  { XOR(cpu, cpu->GetState()->A); return 4; }
	uint8_t Instructions::Instr_0xEE_8ALU_XOR_e8(Cpu* cpu) { XOR(cpu, cpu->CurrentInstruction.lowByte); return 8; }
	uint8_t Instructions::Instr_0xB0_8ALU_OR_B(Cpu* cpu)   { OR(cpu, cpu->GetState()->B); return 4; }
	uint8_t Instructions::Instr_0xB1_8ALU_OR_C(Cpu* cpu)   { OR(cpu, cpu->GetState()->C); return 4; }
	uint8_t Instructions::Instr_0xB2_8ALU_OR_D(Cpu* cpu)   { OR(cpu, cpu->GetState()->D); return 4; }
	uint8_t Instructions::Instr_0xB3_8ALU_OR_E(Cpu* cpu)   { OR(cpu, cpu->GetState()->E); return 4; }
	uint8_t Instructions::Instr_0xB4_8ALU_OR_H(Cpu* cpu)   { OR(cpu, cpu->GetState()->H); return 4; }
	uint8_t Instructions::Instr_0xB5_8ALU_OR_L(Cpu* cpu)   { OR(cpu, cpu->GetState()->L); return 4; }
	uint8_t Instructions::Instr_0xB6_8ALU_OR_HL(Cpu* cpu)  { OR_HL(cpu); return 8; }
	uint8_t Instructions::Instr_0xB7_8ALU_OR_A(Cpu* cpu)   { OR(cpu, cpu->GetState()->A); return 4; }
	uint8_t Instructions::Instr_0xF6_8ALU_OR_e8(Cpu* cpu)  { OR(cpu, cpu->CurrentInstruction.lowByte); return 8; }
	uint8_t Instructions::Instr_0xB8_8ALU_CP_B(Cpu* cpu)   { CP(cpu, cpu->GetState()->B); return 4; }
	uint8_t Instructions::Instr_0xB9_8ALU_CP_C(Cpu* cpu)   { CP(cpu, cpu->GetState()->C); return 4; }
	uint8_t Instructions::Instr_0xBA_8ALU_CP_D(Cpu* cpu)   { CP(cpu, cpu->GetState()->D); return 4; }
	uint8_t Instructions::Instr_0xBB_8ALU_CP_E(Cpu* cpu)   { CP(cpu, cpu->GetState()->E); return 4; }
	uint8_t Instructions::Instr_0xBC_8ALU_CP_H(Cpu* cpu)   { CP(cpu, cpu->GetState()->H); return 4; }
	uint8_t Instructions::Instr_0xBD_8ALU_CP_L(Cpu* cpu)   { CP(cpu, cpu->GetState()->L); return 4; }
	uint8_t Instructions::Instr_0xBE_8ALU_CP_HL(Cpu* cpu)  { CP_HL(cpu); return 8; }

	uint8_t Instructions::Instr_0xBF_8ALU_CP_A(Cpu* cpu)
	{
		// CP A A is a special case where the flags need to be 1 1 0 0
		cpu->SetCPUFlag(FLAG_ZERO, true);
		cpu->SetCPUFlag(FLAG_SUBTRACT, true);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);
		cpu->SetCPUFlag(FLAG_CARRY, false);

		return 4;
	}

	uint8_t Instructions::Instr_0xFE_8ALU_CP_e8(Cpu* cpu) { CP(cpu, cpu->CurrentInstruction.lowByte); return 4; }


	// 16-bit Arithmetic/Logical Instructions
	uint8_t Instructions::Instr_0x03_16ALU_INC_BC(Cpu* cpu) { cpu->GetState()->BC++; return 8; }
	uint8_t Instructions::Instr_0x13_16ALU_INC_DE(Cpu* cpu) { cpu->GetState()->DE++; return 8; }
	uint8_t Instructions::Instr_0x23_16ALU_INC_HL(Cpu* cpu) { cpu->GetState()->HL++; return 8; }
	uint8_t Instructions::Instr_0x33_16ALU_INC_SP(Cpu* cpu) { cpu->GetState()->SP++; return 8; }
	uint8_t Instructions::Instr_0x0B_16ALU_DEC_BC(Cpu* cpu) { cpu->GetState()->BC--; return 8; }
	uint8_t Instructions::Instr_0x1B_16ALU_DEC_DE(Cpu* cpu) { cpu->GetState()->DE--; return 8; }
	uint8_t Instructions::Instr_0x2B_16ALU_DEC_HL(Cpu* cpu) { cpu->GetState()->HL--; return 8; }
	uint8_t Instructions::Instr_0x3B_16ALU_DEC_SP(Cpu* cpu) { cpu->GetState()->SP--; return 8; }
	uint8_t Instructions::Instr_0x09_16ALU_ADD_BC(Cpu* cpu) { ADD_REG16(cpu, cpu->GetState()->BC);return 8; }
	uint8_t Instructions::Instr_0x19_16ALU_ADD_DE(Cpu* cpu) { ADD_REG16(cpu, cpu->GetState()->DE);return 8; }
	uint8_t Instructions::Instr_0x29_16ALU_ADD_HL(Cpu* cpu) { ADD_REG16(cpu, cpu->GetState()->HL);return 8; }
	uint8_t Instructions::Instr_0x39_16ALU_ADD_SP(Cpu* cpu) { ADD_REG16(cpu, cpu->GetState()->SP);return 8; }
	uint8_t Instructions::Instr_0xE8_16ALU_ADD_SP_e8(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		int8_t offset = cpu->CurrentInstruction.lowByte;
		int32_t fullResult = state->SP + offset;
		uint16_t result = static_cast<uint16_t>(fullResult);

		cpu->SetCPUFlag(FLAG_ZERO, false);
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);

		cpu->SetCPUFlag(FLAG_HALF_CARRY, ((state->SP ^ offset ^ (fullResult & 0xFFFF)) & 0x10) == 0x10);
		cpu->SetCPUFlag(FLAG_CARRY, ((state->SP ^ offset ^ (fullResult & 0xFFFF)) & 0x100) == 0x100);

		state->SP = result;

		return 16;
	}


	// 8-bit Shift, Rotate and Bit Instructions
	uint8_t Instructions::Instr_0x07_8SRB_RLCA(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t result = (state->A << 1) | ((state->A >> 7) & 0x1);

		cpu->SetCPUFlag(FLAG_ZERO, false);
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);
		cpu->SetCPUFlag(FLAG_CARRY, ((state->A >> 7) & 0x1) != 0);

		state->A = result;

		return 4;
	}

	uint8_t Instructions::Instr_0x0F_8SRB_RRCA(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t result = ((state->A & 0x1) << 7) | (state->A >> 1);

		cpu->SetCPUFlag(FLAG_ZERO, false);
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);
		cpu->SetCPUFlag(FLAG_CARRY, (state->A & 0x1) != 0);

		state->A = result;

		return 4;
	}

	uint8_t Instructions::Instr_0x17_8SRB_RLA(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t result = (state->A << 1) | (cpu->GetCPUFlag(FLAG_CARRY) ? 1 : 0);
		bool carry = (state->A & 0x80) != 0;
		state->A = result;

		cpu->SetCPUFlag(FLAG_ZERO, false);
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);
		cpu->SetCPUFlag(FLAG_CARRY, carry);

		return 4;
	}

	uint8_t Instructions::Instr_0x1F_8SRB_RRA(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t carry = cpu->GetCPUFlag(FLAG_CARRY) ? 0x80 : 0;
		uint8_t result = carry | (state->A >> 1);

		cpu->SetCPUFlag(FLAG_ZERO, false);
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);
		cpu->SetCPUFlag(FLAG_CARRY, (state->A & 0x1) != 0);

		state->A = result;

		return 4;
	}

	uint8_t Instructions::Instr_0x27_8SRB_DAA(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		int32_t result = state->A;
		if (cpu->GetCPUFlag(FLAG_SUBTRACT))
		{
			if (cpu->GetCPUFlag(FLAG_HALF_CARRY))
			{
				result -= 6;
			}

			if (cpu->GetCPUFlag(FLAG_CARRY))
			{
				result -= 0x60;
			}
		}
		else
		{
			if (cpu->GetCPUFlag(FLAG_HALF_CARRY) || (state->A & 0x0F) > 9)
			{
				result += 6;
			}

			if (cpu->GetCPUFlag(FLAG_CARRY) || result > 0x9F)
			{
				result += 0x60;
			}
		}

		state->A = static_cast<uint8_t>(result);

		if (result > 0xFF)
		{
			cpu->SetCPUFlag(FLAG_CARRY, true);
		}

		cpu->SetCPUFlag(FLAG_ZERO, state->A == 0);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);

		return 4;
	}

	uint8_t Instructions::Instr_0x2F_8SRB_CPL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();
		
		state->A = ~state->A;
		cpu->SetCPUFlag(FLAG_SUBTRACT, true);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, true);

		return 4;
	}

	uint8_t Instructions::Instr_0x37_8SRB_SCF(Cpu* cpu)
	{
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);
		cpu->SetCPUFlag(FLAG_CARRY, true);

		return 4;
	}

	uint8_t Instructions::Instr_0x3F_8SRB_CCF(Cpu* cpu)
	{
		// flip the carry flag
		cpu->GetCPUFlag(FLAG_CARRY) == true ? cpu->SetCPUFlag(FLAG_CARRY, false) : cpu->SetCPUFlag(FLAG_CARRY, true);

		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);

		return 4;
	}


	// 16-bit PREFIX Instructions
	uint8_t Instructions::Instr_16_0xCB00_RLC_B(Cpu* cpu)    { RLC(cpu, cpu->GetState()->B); return 8; }
	uint8_t Instructions::Instr_16_0xCB01_RLC_C(Cpu* cpu)    { RLC(cpu, cpu->GetState()->C); return 8; }
	uint8_t Instructions::Instr_16_0xCB02_RLC_D(Cpu* cpu)    { RLC(cpu, cpu->GetState()->D); return 8; }
	uint8_t Instructions::Instr_16_0xCB03_RLC_E(Cpu* cpu)    { RLC(cpu, cpu->GetState()->E); return 8; }
	uint8_t Instructions::Instr_16_0xCB04_RLC_H(Cpu* cpu)    { RLC(cpu, cpu->GetState()->H); return 8; }
	uint8_t Instructions::Instr_16_0xCB05_RLC_L(Cpu* cpu)    { RLC(cpu, cpu->GetState()->L); return 8; }
	uint8_t Instructions::Instr_16_0xCB06_RLC_HL(Cpu* cpu)   { RLC_HL(cpu); return 16; }
	uint8_t Instructions::Instr_16_0xCB07_RLC_A(Cpu* cpu)    { RLC(cpu, cpu->GetState()->A); return 8; }
	uint8_t Instructions::Instr_16_0xCB08_RRC_B(Cpu* cpu)    { RRC(cpu, cpu->GetState()->B); return 8; }
	uint8_t Instructions::Instr_16_0xCB09_RRC_C(Cpu* cpu)    { RRC(cpu, cpu->GetState()->C); return 8; }
	uint8_t Instructions::Instr_16_0xCB0A_RRC_D(Cpu* cpu)    { RRC(cpu, cpu->GetState()->D); return 8; }
	uint8_t Instructions::Instr_16_0xCB0B_RRC_E(Cpu* cpu)    { RRC(cpu, cpu->GetState()->E); return 8; }
	uint8_t Instructions::Instr_16_0xCB0C_RRC_H(Cpu* cpu)    { RRC(cpu, cpu->GetState()->H); return 8; }
	uint8_t Instructions::Instr_16_0xCB0D_RRC_L(Cpu* cpu)    { RRC(cpu, cpu->GetState()->L); return 8; }
	uint8_t Instructions::Instr_16_0xCB0E_RRC_HL(Cpu* cpu)   { RRC_HL(cpu); return 16; }
	uint8_t Instructions::Instr_16_0xCB0F_RRC_A(Cpu* cpu)    { RRC(cpu, cpu->GetState()->A); return 8; }
	uint8_t Instructions::Instr_16_0xCB10_RL_B(Cpu* cpu)     { RL(cpu, cpu->GetState()->B); return 8; }
	uint8_t Instructions::Instr_16_0xCB11_RL_C(Cpu* cpu)     { RL(cpu, cpu->GetState()->C); return 8; }
	uint8_t Instructions::Instr_16_0xCB12_RL_D(Cpu* cpu)     { RL(cpu, cpu->GetState()->D); return 8; }
	uint8_t Instructions::Instr_16_0xCB13_RL_E(Cpu* cpu)     { RL(cpu, cpu->GetState()->E); return 8; }
	uint8_t Instructions::Instr_16_0xCB14_RL_H(Cpu* cpu)     { RL(cpu, cpu->GetState()->H); return 8; }
	uint8_t Instructions::Instr_16_0xCB15_RL_L(Cpu* cpu)     { RL(cpu, cpu->GetState()->L); return 8; }
	uint8_t Instructions::Instr_16_0xCB16_RL_HL(Cpu* cpu)    { RL_HL(cpu); return 16; }
	uint8_t Instructions::Instr_16_0xCB17_RL_A(Cpu* cpu)     { RL(cpu, cpu->GetState()->A); return 8; }
	uint8_t Instructions::Instr_16_0xCB18_RR_B(Cpu* cpu)     { RR(cpu, cpu->GetState()->B); return 8; }
	uint8_t Instructions::Instr_16_0xCB19_RR_C(Cpu* cpu)     { RR(cpu, cpu->GetState()->C); return 8; }
	uint8_t Instructions::Instr_16_0xCB1A_RR_D(Cpu* cpu)     { RR(cpu, cpu->GetState()->D); return 8; }
	uint8_t Instructions::Instr_16_0xCB1B_RR_E(Cpu* cpu)     { RR(cpu, cpu->GetState()->E); return 8; }
	uint8_t Instructions::Instr_16_0xCB1C_RR_H(Cpu* cpu)     { RR(cpu, cpu->GetState()->H); return 8; }
	uint8_t Instructions::Instr_16_0xCB1D_RR_L(Cpu* cpu)     { RR(cpu, cpu->GetState()->L); return 8; }
	uint8_t Instructions::Instr_16_0xCB1E_RR_HL(Cpu* cpu)    { RR_HL(cpu); return 16; }
	uint8_t Instructions::Instr_16_0xCB1F_RR_A(Cpu* cpu)     { RR(cpu, cpu->GetState()->A); return 8; }
	uint8_t Instructions::Instr_16_0xCB20_SLA_B(Cpu* cpu)    { SLA(cpu, cpu->GetState()->B); return 8; }
	uint8_t Instructions::Instr_16_0xCB21_SLA_C(Cpu* cpu)    { SLA(cpu, cpu->GetState()->C); return 8; }
	uint8_t Instructions::Instr_16_0xCB22_SLA_D(Cpu* cpu)    { SLA(cpu, cpu->GetState()->D); return 8; }
	uint8_t Instructions::Instr_16_0xCB23_SLA_E(Cpu* cpu)    { SLA(cpu, cpu->GetState()->E); return 8; }
	uint8_t Instructions::Instr_16_0xCB24_SLA_H(Cpu* cpu)    { SLA(cpu, cpu->GetState()->H); return 8; }
	uint8_t Instructions::Instr_16_0xCB25_SLA_L(Cpu* cpu)    { SLA(cpu, cpu->GetState()->L); return 8; }
	uint8_t Instructions::Instr_16_0xCB26_SLA_HL(Cpu* cpu)   { SLA_HL(cpu); return 16; }
	uint8_t Instructions::Instr_16_0xCB27_SLA_A(Cpu* cpu)    { SLA(cpu, cpu->GetState()->A); return 8; }
	uint8_t Instructions::Instr_16_0xCB28_SRA_B(Cpu* cpu)    { SRA(cpu, cpu->GetState()->B); return 8; }
	uint8_t Instructions::Instr_16_0xCB29_SRA_C(Cpu* cpu)    { SRA(cpu, cpu->GetState()->C); return 8; }
	uint8_t Instructions::Instr_16_0xCB2A_SRA_D(Cpu* cpu)    { SRA(cpu, cpu->GetState()->D); return 8; }
	uint8_t Instructions::Instr_16_0xCB2B_SRA_E(Cpu* cpu)    { SRA(cpu, cpu->GetState()->E); return 8; }
	uint8_t Instructions::Instr_16_0xCB2C_SRA_H(Cpu* cpu)    { SRA(cpu, cpu->GetState()->H); return 8; }
	uint8_t Instructions::Instr_16_0xCB2D_SRA_L(Cpu* cpu)    { SRA(cpu, cpu->GetState()->L); return 8; }
	uint8_t Instructions::Instr_16_0xCB2E_SRA_HL(Cpu* cpu)   { SRA_HL(cpu); return 16; }
	uint8_t Instructions::Instr_16_0xCB2F_SRA_A(Cpu* cpu)    { SRA(cpu, cpu->GetState()->A); return 8; }
	uint8_t Instructions::Instr_16_0xCB30_SWAP_B(Cpu* cpu)   { SWAP(cpu, cpu->GetState()->B); return 8; }
	uint8_t Instructions::Instr_16_0xCB31_SWAP_C(Cpu* cpu)   { SWAP(cpu, cpu->GetState()->C); return 8; }
	uint8_t Instructions::Instr_16_0xCB32_SWAP_D(Cpu* cpu)   { SWAP(cpu, cpu->GetState()->D); return 8; }
	uint8_t Instructions::Instr_16_0xCB33_SWAP_E(Cpu* cpu)   { SWAP(cpu, cpu->GetState()->E); return 8; }
	uint8_t Instructions::Instr_16_0xCB34_SWAP_H(Cpu* cpu)   { SWAP(cpu, cpu->GetState()->H); return 8; }
	uint8_t Instructions::Instr_16_0xCB35_SWAP_L(Cpu* cpu)   { SWAP(cpu, cpu->GetState()->L); return 8; }
	uint8_t Instructions::Instr_16_0xCB36_SWAP_HL(Cpu* cpu)  { SWAP_HL(cpu); return 16; }
	uint8_t Instructions::Instr_16_0xCB37_SWAP_A(Cpu* cpu)   { SWAP(cpu, cpu->GetState()->A); return 8; }
	uint8_t Instructions::Instr_16_0xCB38_SRL_B(Cpu* cpu)    { SRL(cpu, cpu->GetState()->B); return 8; }
	uint8_t Instructions::Instr_16_0xCB39_SRL_C(Cpu* cpu)    { SRL(cpu, cpu->GetState()->C); return 8; }
	uint8_t Instructions::Instr_16_0xCB3A_SRL_D(Cpu* cpu)    { SRL(cpu, cpu->GetState()->D); return 8; }
	uint8_t Instructions::Instr_16_0xCB3B_SRL_E(Cpu* cpu)    { SRL(cpu, cpu->GetState()->E); return 8; }
	uint8_t Instructions::Instr_16_0xCB3C_SRL_H(Cpu* cpu)    { SRL(cpu, cpu->GetState()->H); return 8; }
	uint8_t Instructions::Instr_16_0xCB3D_SRL_L(Cpu* cpu)    { SRL(cpu, cpu->GetState()->L); return 8; }
	uint8_t Instructions::Instr_16_0xCB3E_SRL_HL(Cpu* cpu)   { SRL_HL(cpu); return 16; }
	uint8_t Instructions::Instr_16_0xCB3F_SRL_A(Cpu* cpu)    { SRL(cpu, cpu->GetState()->A); return 8; }
	uint8_t Instructions::Instr_16_0xCB40_BIT_0_B(Cpu* cpu)  { BIT(cpu, cpu->GetState()->B, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCB41_BIT_0_C(Cpu* cpu)  { BIT(cpu, cpu->GetState()->C, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCB42_BIT_0_D(Cpu* cpu)  { BIT(cpu, cpu->GetState()->D, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCB43_BIT_0_E(Cpu* cpu)  { BIT(cpu, cpu->GetState()->E, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCB44_BIT_0_H(Cpu* cpu)  { BIT(cpu, cpu->GetState()->H, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCB45_BIT_0_L(Cpu* cpu)  { BIT(cpu, cpu->GetState()->L, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCB46_BIT_0_HL(Cpu* cpu) { BIT_HL(cpu, 0); return 12; }
	uint8_t Instructions::Instr_16_0xCB47_BIT_0_A(Cpu* cpu)  { BIT(cpu, cpu->GetState()->A, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCB48_BIT_1_B(Cpu* cpu)  { BIT(cpu, cpu->GetState()->B, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCB49_BIT_1_C(Cpu* cpu)  { BIT(cpu, cpu->GetState()->C, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCB4A_BIT_1_D(Cpu* cpu)  { BIT(cpu, cpu->GetState()->D, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCB4B_BIT_1_E(Cpu* cpu)  { BIT(cpu, cpu->GetState()->E, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCB4C_BIT_1_H(Cpu* cpu)  { BIT(cpu, cpu->GetState()->H, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCB4D_BIT_1_L(Cpu* cpu)  { BIT(cpu, cpu->GetState()->L, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCB4E_BIT_1_HL(Cpu* cpu) { BIT_HL(cpu, 1); return 12; }
	uint8_t Instructions::Instr_16_0xCB4F_BIT_1_A(Cpu* cpu)  { BIT(cpu, cpu->GetState()->A, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCB50_BIT_2_B(Cpu* cpu)  { BIT(cpu, cpu->GetState()->B, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCB51_BIT_2_C(Cpu* cpu)  { BIT(cpu, cpu->GetState()->C, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCB52_BIT_2_D(Cpu* cpu)  { BIT(cpu, cpu->GetState()->D, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCB53_BIT_2_E(Cpu* cpu)  { BIT(cpu, cpu->GetState()->E, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCB54_BIT_2_H(Cpu* cpu)  { BIT(cpu, cpu->GetState()->H, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCB55_BIT_2_L(Cpu* cpu)  { BIT(cpu, cpu->GetState()->L, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCB56_BIT_2_HL(Cpu* cpu) { BIT_HL(cpu, 2); return 12; }
	uint8_t Instructions::Instr_16_0xCB57_BIT_2_A(Cpu* cpu)  { BIT(cpu, cpu->GetState()->A, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCB58_BIT_3_B(Cpu* cpu)  { BIT(cpu, cpu->GetState()->B, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCB59_BIT_3_C(Cpu* cpu)  { BIT(cpu, cpu->GetState()->C, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCB5A_BIT_3_D(Cpu* cpu)  { BIT(cpu, cpu->GetState()->D, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCB5B_BIT_3_E(Cpu* cpu)  { BIT(cpu, cpu->GetState()->E, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCB5C_BIT_3_H(Cpu* cpu)  { BIT(cpu, cpu->GetState()->H, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCB5D_BIT_3_L(Cpu* cpu)  { BIT(cpu, cpu->GetState()->L, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCB5E_BIT_3_HL(Cpu* cpu) { BIT_HL(cpu, 3); return 12; }
	uint8_t Instructions::Instr_16_0xCB5F_BIT_3_A(Cpu* cpu)  { BIT(cpu, cpu->GetState()->A, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCB60_BIT_4_B(Cpu* cpu)  { BIT(cpu, cpu->GetState()->B, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCB61_BIT_4_C(Cpu* cpu)  { BIT(cpu, cpu->GetState()->C, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCB62_BIT_4_D(Cpu* cpu)  { BIT(cpu, cpu->GetState()->D, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCB63_BIT_4_E(Cpu* cpu)  { BIT(cpu, cpu->GetState()->E, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCB64_BIT_4_H(Cpu* cpu)  { BIT(cpu, cpu->GetState()->H, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCB65_BIT_4_L(Cpu* cpu)  { BIT(cpu, cpu->GetState()->L, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCB66_BIT_4_HL(Cpu* cpu) { BIT_HL(cpu, 4); return 12; }
	uint8_t Instructions::Instr_16_0xCB67_BIT_4_A(Cpu* cpu)  { BIT(cpu, cpu->GetState()->A, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCB68_BIT_5_B(Cpu* cpu)  { BIT(cpu, cpu->GetState()->B, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCB69_BIT_5_C(Cpu* cpu)  { BIT(cpu, cpu->GetState()->C, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCB6A_BIT_5_D(Cpu* cpu)  { BIT(cpu, cpu->GetState()->D, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCB6B_BIT_5_E(Cpu* cpu)  { BIT(cpu, cpu->GetState()->E, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCB6C_BIT_5_H(Cpu* cpu)  { BIT(cpu, cpu->GetState()->H, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCB6D_BIT_5_L(Cpu* cpu)  { BIT(cpu, cpu->GetState()->L, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCB6E_BIT_5_HL(Cpu* cpu) { BIT_HL(cpu, 5); return 12; }
	uint8_t Instructions::Instr_16_0xCB6F_BIT_5_A(Cpu* cpu)  { BIT(cpu, cpu->GetState()->A, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCB70_BIT_6_B(Cpu* cpu)  { BIT(cpu, cpu->GetState()->B, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCB71_BIT_6_C(Cpu* cpu)  { BIT(cpu, cpu->GetState()->C, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCB72_BIT_6_D(Cpu* cpu)  { BIT(cpu, cpu->GetState()->D, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCB73_BIT_6_E(Cpu* cpu)  { BIT(cpu, cpu->GetState()->E, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCB74_BIT_6_H(Cpu* cpu)  { BIT(cpu, cpu->GetState()->H, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCB75_BIT_6_L(Cpu* cpu)  { BIT(cpu, cpu->GetState()->L, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCB76_BIT_6_HL(Cpu* cpu) { BIT_HL(cpu, 6); return 12; }
	uint8_t Instructions::Instr_16_0xCB77_BIT_6_A(Cpu* cpu)  { BIT(cpu, cpu->GetState()->A, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCB78_BIT_7_B(Cpu* cpu)  { BIT(cpu, cpu->GetState()->B, 7); return 8; }
	uint8_t Instructions::Instr_16_0xCB79_BIT_7_C(Cpu* cpu)  { BIT(cpu, cpu->GetState()->C, 7); return 8; }
	uint8_t Instructions::Instr_16_0xCB7A_BIT_7_D(Cpu* cpu)  { BIT(cpu, cpu->GetState()->D, 7); return 8; }
	uint8_t Instructions::Instr_16_0xCB7B_BIT_7_E(Cpu* cpu)  { BIT(cpu, cpu->GetState()->E, 7); return 8; }
	uint8_t Instructions::Instr_16_0xCB7C_BIT_7_H(Cpu* cpu)  { BIT(cpu, cpu->GetState()->H, 7); return 8; }
	uint8_t Instructions::Instr_16_0xCB7D_BIT_7_L(Cpu* cpu)  { BIT(cpu, cpu->GetState()->L, 7); return 8; }
	uint8_t Instructions::Instr_16_0xCB7E_BIT_7_HL(Cpu* cpu) { BIT_HL(cpu, 7); return 12; }
	uint8_t Instructions::Instr_16_0xCB7F_BIT_7_A(Cpu* cpu)  { BIT(cpu, cpu->GetState()->A, 7); return 8; }
	uint8_t Instructions::Instr_16_0xCB80_RES_0_B(Cpu* cpu)  { RES(cpu->GetState()->B, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCB81_RES_0_C(Cpu* cpu)  { RES(cpu->GetState()->C, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCB82_RES_0_D(Cpu* cpu)  { RES(cpu->GetState()->D, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCB83_RES_0_E(Cpu* cpu)  { RES(cpu->GetState()->E, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCB84_RES_0_H(Cpu* cpu)  { RES(cpu->GetState()->H, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCB85_RES_0_L(Cpu* cpu)  { RES(cpu->GetState()->L, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCB86_RES_0_H(Cpu* cpu)  { RES_HL(cpu, 0); return 16; }
	uint8_t Instructions::Instr_16_0xCB87_RES_0_A(Cpu* cpu)  { RES(cpu->GetState()->A, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCB88_RES_1_B(Cpu* cpu)  { RES(cpu->GetState()->B, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCB89_RES_1_C(Cpu* cpu)  { RES(cpu->GetState()->C, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCB8A_RES_1_D(Cpu* cpu)  { RES(cpu->GetState()->D, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCB8B_RES_1_E(Cpu* cpu)  { RES(cpu->GetState()->E, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCB8C_RES_1_H(Cpu* cpu)  { RES(cpu->GetState()->H, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCB8D_RES_1_L(Cpu* cpu)  { RES(cpu->GetState()->L, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCB8E_RES_1_HL(Cpu* cpu) { RES_HL(cpu, 1); return 16; }
	uint8_t Instructions::Instr_16_0xCB8F_RES_1_A(Cpu* cpu)  { RES(cpu->GetState()->A, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCB90_RES_2_B(Cpu* cpu)  { RES(cpu->GetState()->B, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCB91_RES_2_C(Cpu* cpu)  { RES(cpu->GetState()->C, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCB92_RES_2_D(Cpu* cpu)  { RES(cpu->GetState()->D, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCB93_RES_2_E(Cpu* cpu)  { RES(cpu->GetState()->E, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCB94_RES_2_H(Cpu* cpu)  { RES(cpu->GetState()->H, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCB95_RES_2_L(Cpu* cpu)  { RES(cpu->GetState()->L, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCB96_RES_2_HL(Cpu* cpu) { RES_HL(cpu, 2); return 16; }
	uint8_t Instructions::Instr_16_0xCB97_RES_2_A(Cpu* cpu)  { RES(cpu->GetState()->A, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCB98_RES_3_B(Cpu* cpu)  { RES(cpu->GetState()->B, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCB99_RES_3_C(Cpu* cpu)  { RES(cpu->GetState()->C, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCB9A_RES_3_D(Cpu* cpu)  { RES(cpu->GetState()->D, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCB9B_RES_3_E(Cpu* cpu)  { RES(cpu->GetState()->E, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCB9C_RES_3_H(Cpu* cpu)  { RES(cpu->GetState()->H, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCB9D_RES_3_L(Cpu* cpu)  { RES(cpu->GetState()->L, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCB9E_RES_3_HL(Cpu* cpu) { RES_HL(cpu, 3); return 16; }
	uint8_t Instructions::Instr_16_0xCB9F_RES_3_A(Cpu* cpu)  { RES(cpu->GetState()->A, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCBA0_RES_4_B(Cpu* cpu)  { RES(cpu->GetState()->B, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCBA1_RES_4_C(Cpu* cpu)  { RES(cpu->GetState()->C, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCBA2_RES_4_D(Cpu* cpu)  { RES(cpu->GetState()->D, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCBA3_RES_4_E(Cpu* cpu)  { RES(cpu->GetState()->E, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCBA4_RES_4_H(Cpu* cpu)  { RES(cpu->GetState()->H, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCBA5_RES_4_L(Cpu* cpu)  { RES(cpu->GetState()->L, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCBA6_RES_4_HL(Cpu* cpu) { RES_HL(cpu, 4); return 16; }
	uint8_t Instructions::Instr_16_0xCBA7_RES_4_A(Cpu* cpu)  { RES(cpu->GetState()->A, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCBA8_RES_5_B(Cpu* cpu)  { RES(cpu->GetState()->B, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCBA9_RES_5_C(Cpu* cpu)  { RES(cpu->GetState()->C, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCBAA_RES_5_D(Cpu* cpu)  { RES(cpu->GetState()->D, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCBAB_RES_5_E(Cpu* cpu)  { RES(cpu->GetState()->E, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCBAC_RES_5_H(Cpu* cpu)  { RES(cpu->GetState()->H, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCBAD_RES_5_L(Cpu* cpu)  { RES(cpu->GetState()->L, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCBAE_RES_5_HL(Cpu* cpu) { RES_HL(cpu, 5); return 16; }
	uint8_t Instructions::Instr_16_0xCBAF_RES_5_A(Cpu* cpu)  { RES(cpu->GetState()->A, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCBB0_RES_6_B(Cpu* cpu)  { RES(cpu->GetState()->B, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCBB1_RES_6_C(Cpu* cpu)  { RES(cpu->GetState()->C, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCBB2_RES_6_D(Cpu* cpu)  { RES(cpu->GetState()->D, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCBB3_RES_6_E(Cpu* cpu)  { RES(cpu->GetState()->E, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCBB4_RES_6_H(Cpu* cpu)  { RES(cpu->GetState()->H, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCBB5_RES_6_L(Cpu* cpu)  { RES(cpu->GetState()->L, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCBB6_RES_6_HL(Cpu* cpu) { RES_HL(cpu, 6); return 16; }
	uint8_t Instructions::Instr_16_0xCBB7_RES_6_A(Cpu* cpu)  { RES(cpu->GetState()->A, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCBB8_RES_7_B(Cpu* cpu)  { RES(cpu->GetState()->B, 7); return 8; }
	uint8_t Instructions::Instr_16_0xCBB9_RES_7_C(Cpu* cpu)  { RES(cpu->GetState()->C, 7); return 8; }
	uint8_t Instructions::Instr_16_0xCBBA_RES_7_D(Cpu* cpu)  { RES(cpu->GetState()->D, 7); return 8; }
	uint8_t Instructions::Instr_16_0xCBBB_RES_7_E(Cpu* cpu)  { RES(cpu->GetState()->E, 7); return 8; }
	uint8_t Instructions::Instr_16_0xCBBC_RES_7_H(Cpu* cpu)  { RES(cpu->GetState()->H, 7); return 8; }
	uint8_t Instructions::Instr_16_0xCBBD_RES_7_L(Cpu* cpu)  { RES(cpu->GetState()->L, 7); return 8; }
	uint8_t Instructions::Instr_16_0xCBBE_RES_7_HL(Cpu* cpu) { RES_HL(cpu, 7); return 16; }
	uint8_t Instructions::Instr_16_0xCBBF_RES_7_A(Cpu* cpu)  { RES(cpu->GetState()->A, 7); return 8; }
	uint8_t Instructions::Instr_16_0xCBC0_SET_0_B(Cpu* cpu)  { SET(cpu->GetState()->B, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCBC1_SET_0_C(Cpu* cpu)  { SET(cpu->GetState()->C, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCBC2_SET_0_D(Cpu* cpu)  { SET(cpu->GetState()->D, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCBC3_SET_0_E(Cpu* cpu)  { SET(cpu->GetState()->E, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCBC4_SET_0_H(Cpu* cpu)  { SET(cpu->GetState()->H, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCBC5_SET_0_L(Cpu* cpu)  { SET(cpu->GetState()->L, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCBC6_SET_0_HL(Cpu* cpu) { SET_HL(cpu, 0); return 16; }
	uint8_t Instructions::Instr_16_0xCBC7_SET_0_A(Cpu* cpu)  { SET(cpu->GetState()->A, 0); return 8; }
	uint8_t Instructions::Instr_16_0xCBC8_SET_1_B(Cpu* cpu)  { SET(cpu->GetState()->B, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCBC9_SET_1_C(Cpu* cpu)  { SET(cpu->GetState()->C, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCBCA_SET_1_D(Cpu* cpu)  { SET(cpu->GetState()->D, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCBCB_SET_1_E(Cpu* cpu)  { SET(cpu->GetState()->E, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCBCC_SET_1_H(Cpu* cpu)  { SET(cpu->GetState()->H, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCBCD_SET_1_L(Cpu* cpu)  { SET(cpu->GetState()->L, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCBCE_SET_1_HL(Cpu* cpu) { SET_HL(cpu, 1); return 16; }
	uint8_t Instructions::Instr_16_0xCBCF_SET_1_A(Cpu* cpu)  { SET(cpu->GetState()->A, 1); return 8; }
	uint8_t Instructions::Instr_16_0xCBD0_SET_2_B(Cpu* cpu)  { SET(cpu->GetState()->B, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCBD1_SET_2_C(Cpu* cpu)  { SET(cpu->GetState()->C, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCBD2_SET_2_D(Cpu* cpu)  { SET(cpu->GetState()->D, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCBD3_SET_2_E(Cpu* cpu)  { SET(cpu->GetState()->E, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCBD4_SET_2_H(Cpu* cpu)  { SET(cpu->GetState()->H, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCBD5_SET_2_L(Cpu* cpu)  { SET(cpu->GetState()->L, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCBD6_SET_2_HL(Cpu* cpu) { SET_HL(cpu, 2); return 16; }
	uint8_t Instructions::Instr_16_0xCBD7_SET_2_A(Cpu* cpu)  { SET(cpu->GetState()->A, 2); return 8; }
	uint8_t Instructions::Instr_16_0xCBD8_SET_3_B(Cpu* cpu)  { SET(cpu->GetState()->B, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCBD9_SET_3_C(Cpu* cpu)  { SET(cpu->GetState()->C, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCBDA_SET_3_D(Cpu* cpu)  { SET(cpu->GetState()->D, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCBDB_SET_3_E(Cpu* cpu)  { SET(cpu->GetState()->E, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCBDC_SET_3_H(Cpu* cpu)  { SET(cpu->GetState()->H, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCBDD_SET_3_L(Cpu* cpu)  { SET(cpu->GetState()->L, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCBDE_SET_3_HL(Cpu* cpu) { SET_HL(cpu, 3); return 16; }
	uint8_t Instructions::Instr_16_0xCBDF_SET_3_A(Cpu* cpu)  { SET(cpu->GetState()->A, 3); return 8; }
	uint8_t Instructions::Instr_16_0xCBE0_SET_4_B(Cpu* cpu)  { SET(cpu->GetState()->B, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCBE1_SET_4_C(Cpu* cpu)  { SET(cpu->GetState()->C, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCBE2_SET_4_D(Cpu* cpu)  { SET(cpu->GetState()->D, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCBE3_SET_4_E(Cpu* cpu)  { SET(cpu->GetState()->E, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCBE4_SET_4_H(Cpu* cpu)  { SET(cpu->GetState()->H, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCBE5_SET_4_L(Cpu* cpu)  { SET(cpu->GetState()->L, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCBE6_SET_4_HL(Cpu* cpu) { SET_HL(cpu, 4); return 16; }
	uint8_t Instructions::Instr_16_0xCBE7_SET_4_A(Cpu* cpu)  { SET(cpu->GetState()->A, 4); return 8; }
	uint8_t Instructions::Instr_16_0xCBE8_SET_5_B(Cpu* cpu)  { SET(cpu->GetState()->B, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCBE9_SET_5_C(Cpu* cpu)  { SET(cpu->GetState()->C, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCBEA_SET_5_D(Cpu* cpu)  { SET(cpu->GetState()->D, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCBEB_SET_5_E(Cpu* cpu)  { SET(cpu->GetState()->E, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCBEC_SET_5_H(Cpu* cpu)  { SET(cpu->GetState()->H, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCBED_SET_5_L(Cpu* cpu)  { SET(cpu->GetState()->L, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCBEE_SET_5_HL(Cpu* cpu) { SET_HL(cpu, 5); return 16; }
	uint8_t Instructions::Instr_16_0xCBEF_SET_5_A(Cpu* cpu)  { SET(cpu->GetState()->A, 5); return 8; }
	uint8_t Instructions::Instr_16_0xCBF0_SET_6_B(Cpu* cpu)  { SET(cpu->GetState()->B, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCBF1_SET_6_C(Cpu* cpu)  { SET(cpu->GetState()->C, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCBF2_SET_6_D(Cpu* cpu)  { SET(cpu->GetState()->D, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCBF3_SET_6_E(Cpu* cpu)  { SET(cpu->GetState()->E, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCBF4_SET_6_H(Cpu* cpu)  { SET(cpu->GetState()->H, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCBF5_SET_6_L(Cpu* cpu)  { SET(cpu->GetState()->L, 6); return 8; }
	uint8_t Instructions::Instr_16_0xCBF6_SET_6_HL(Cpu* cpu) { SET_HL(cpu, 6); return 16; }
	uint8_t Instructions::Instr_16_0xCBF7_SET_6_A(Cpu* cpu)  { SET(cpu->GetState()->A, 6); return 8; } 
	uint8_t Instructions::Instr_16_0xCBF8_SET_7_B(Cpu* cpu)  { SET(cpu->GetState()->B, 7); return 8; } 
	uint8_t Instructions::Instr_16_0xCBF9_SET_7_C(Cpu* cpu)  { SET(cpu->GetState()->C, 7); return 8; } 
	uint8_t Instructions::Instr_16_0xCBFA_SET_7_D(Cpu* cpu)  { SET(cpu->GetState()->D, 7); return 8; } 
	uint8_t Instructions::Instr_16_0xCBFB_SET_7_E(Cpu* cpu)  { SET(cpu->GetState()->E, 7); return 8; } 
	uint8_t Instructions::Instr_16_0xCBFC_SET_7_H(Cpu* cpu)  { SET(cpu->GetState()->H, 7); return 8; } 
	uint8_t Instructions::Instr_16_0xCBFD_SET_7_L(Cpu* cpu)  { SET(cpu->GetState()->L, 7); return 8; } 
	uint8_t Instructions::Instr_16_0xCBFE_SET_7_HL(Cpu* cpu) { SET_HL(cpu, 7); return 16; }
	uint8_t Instructions::Instr_16_0xCBFF_SET_7_A(Cpu* cpu)  { SET(cpu->GetState()->A, 7); return 8; }


	// helpers
	void Instructions::INC(Cpu* cpu, uint8_t& reg)
	{
		reg++;

		cpu->SetCPUFlag(FLAG_ZERO, (reg == 0));
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, (reg & 0x0F) == 0x00);
	}

	void Instructions::INC_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		INC(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::DEC(Cpu* cpu, uint8_t& reg)
	{
		reg--;

		cpu->SetCPUFlag(FLAG_ZERO, (reg == 0));
		cpu->SetCPUFlag(FLAG_SUBTRACT, true);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, (reg & 0x0F) == 0x0F);
	}

	void Instructions::DEC_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		DEC(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::ADD(Cpu* cpu, uint8_t& reg)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint16_t fullResult = state->A + reg;
		uint8_t result = static_cast<uint8_t>(fullResult);

		// Update flags
		cpu->SetCPUFlag(FLAG_ZERO, result == 0);
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, ((state->A & 0x0F) + (reg & 0x0F) > 0x0F));
		cpu->SetCPUFlag(FLAG_CARRY, (fullResult > 0xFF));

		state->A = result;
	}

	void Instructions::ADD_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		ADD(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::ADC(Cpu* cpu, uint8_t& reg)
	{
		Cpu::sCPUState* state = cpu->GetState();

		bool carry = cpu->GetCPUFlag(FLAG_CARRY);
		uint16_t fullResult = state->A + reg + (carry ? 1 : 0);
		uint8_t result = static_cast<uint8_t>(fullResult);

		// Update flags
		cpu->SetCPUFlag(FLAG_ZERO, result == 0);
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, (state->A & 0xF) + (reg & 0xF) + (carry ? 1 : 0) > 0xF);
		cpu->SetCPUFlag(FLAG_CARRY, (fullResult > 0xFF));

		state->A = result;
	}

	void Instructions::ADC_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		ADC(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::SUB(Cpu* cpu, uint8_t& reg)
	{
		Cpu::sCPUState* state = cpu->GetState();
		uint8_t result = state->A - reg;

		// Update flags
		cpu->SetCPUFlag(FLAG_ZERO, result == 0);
		cpu->SetCPUFlag(FLAG_SUBTRACT, true);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, (state->A & 0x0F) - (reg & 0x0F) < 0);
		cpu->SetCPUFlag(FLAG_CARRY, state->A < reg);

		state->A = result;
	}

	void Instructions::SUB_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		SUB(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::SBC(Cpu* cpu, uint8_t& reg)
	{
		Cpu::sCPUState* state = cpu->GetState();

		bool carry = cpu->GetCPUFlag(FLAG_CARRY);
		int32_t fullResult = state->A - reg - (carry ? 1 : 0);
		uint8_t result = static_cast<uint8_t>(fullResult);

		cpu->SetCPUFlag(FLAG_ZERO, result == 0);
		cpu->SetCPUFlag(FLAG_SUBTRACT, true);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, (state->A & 0xF) - (reg & 0xF) - carry < 0);
		cpu->SetCPUFlag(FLAG_CARRY, fullResult < 0);

		state->A = result;
	}

	void Instructions::SBC_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		SBC(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::AND(Cpu* cpu, uint8_t& reg)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->A = state->A & reg;

		cpu->SetCPUFlag(FLAG_ZERO, state->A == 0);
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, true);
		cpu->SetCPUFlag(FLAG_CARRY, false);
	}

	void Instructions::AND_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		AND(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::XOR(Cpu* cpu, uint8_t& reg)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->A = state->A ^ reg;

		cpu->SetCPUFlag(FLAG_ZERO, state->A == 0);
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);
		cpu->SetCPUFlag(FLAG_CARRY, false);
	}

	void Instructions::XOR_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		XOR(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::OR(Cpu* cpu, uint8_t& reg)
	{
		Cpu::sCPUState* state = cpu->GetState();
		state->A = state->A | reg;

		cpu->SetCPUFlag(FLAG_ZERO, state->A == 0);
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);
		cpu->SetCPUFlag(FLAG_CARRY, false);
	}

	void Instructions::OR_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		OR(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::CP(Cpu* cpu, uint8_t& reg)
	{
		Cpu::sCPUState* state = cpu->GetState();

		cpu->SetCPUFlag(FLAG_ZERO, state->A == reg);
		cpu->SetCPUFlag(FLAG_SUBTRACT, true);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, (state->A & 0xF) - (reg & 0xF) < 0);
		cpu->SetCPUFlag(FLAG_CARRY, state->A < reg);
	}

	void Instructions::CP_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		CP(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::ADD_REG16(Cpu* cpu, uint16_t& reg)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint32_t fullResult = state->HL + reg;
		uint16_t result = static_cast<uint16_t>(fullResult);

		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, (state->HL ^ reg ^ (fullResult & 0xFFFF)) & 0x1000);
		cpu->SetCPUFlag(FLAG_CARRY, fullResult > 0xFFFF);

		state->HL = result;
	}


	// prefix helpers
	void Instructions::RLC(Cpu* cpu, uint8_t& reg)
	{
		uint8_t result = (reg << 1) | ((reg >> 7) & 0x1);

		cpu->SetCPUFlag(FLAG_ZERO, result == 0);
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);
		cpu->SetCPUFlag(FLAG_CARRY, ((reg >> 7) & 0x1) != 0);

		reg = result;
	}

	void Instructions::RLC_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		RLC(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::RRC(Cpu* cpu, uint8_t& reg)
	{
		cpu->SetCPUFlag(FLAG_CARRY, (reg & 0x01) != 0);

		reg = (reg >> 1) | (cpu->GetCPUFlag(FLAG_CARRY) ? 0x80 : 0x00);

		cpu->SetCPUFlag(FLAG_ZERO, (reg == 0));
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);
	}

	void Instructions::RRC_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		RRC(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::RL(Cpu* cpu, uint8_t& reg)
	{
		// Rotate left through carry
		bool carry = cpu->GetCPUFlag(FLAG_CARRY);
		uint8_t temp = (reg << 1) | (carry ? 1 : 0);
		carry = (reg & 0x80) != 0;
		reg = temp;

		// Update flags
		cpu->SetCPUFlag(FLAG_ZERO, (reg == 0));
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);
		cpu->SetCPUFlag(FLAG_CARRY, carry);
	}

	void Instructions::RL_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		RL(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::RR(Cpu* cpu, uint8_t& reg)
	{
		uint8_t carry = cpu->GetCPUFlag(FLAG_CARRY) ? 0x80 : 0;
		uint8_t result = carry | (reg >> 1);

		cpu->SetCPUFlag(FLAG_ZERO, result == 0);
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);
		cpu->SetCPUFlag(FLAG_CARRY, (reg & 0x1) != 0);

		reg = result;
	}

	void Instructions::RR_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		RR(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::SLA(Cpu* cpu, uint8_t& reg)
	{
		cpu->SetCPUFlag(FLAG_CARRY, (reg & 0x80) != 0);

		reg = reg << 1;

		cpu->SetCPUFlag(FLAG_ZERO, (reg == 0));
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);
	}

	void Instructions::SLA_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		SLA(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::SRA(Cpu* cpu, uint8_t& reg)
	{
		uint8_t result = (reg & 0x80) | (reg >> 1);

		cpu->SetCPUFlag(FLAG_ZERO, result == 0);
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);
		cpu->SetCPUFlag(FLAG_CARRY, ((reg & 0x1) != 0));

		reg = result;
	}

	void Instructions::SRA_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		SRA(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::SWAP(Cpu* cpu, uint8_t& reg)
	{
		reg = ((reg & 0x0F) << 4) | ((reg & 0xF0) >> 4);

		cpu->SetCPUFlag(FLAG_ZERO, (reg == 0));
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);
		cpu->SetCPUFlag(FLAG_CARRY, false);
	}

	void Instructions::SWAP_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		SWAP(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::SRL(Cpu* cpu, uint8_t& reg)
	{
		cpu->SetCPUFlag(FLAG_CARRY, (reg & 0x01) != 0);

		reg = reg >> 1;

		cpu->SetCPUFlag(FLAG_ZERO, (reg == 0));
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, false);
	}

	void Instructions::SRL_HL(Cpu* cpu)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		SRL(cpu, value);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::BIT(Cpu* cpu, uint8_t& reg, uint8_t bit)
	{
		cpu->SetCPUFlag(FLAG_ZERO, (reg & (1 << bit)) == 0);
		cpu->SetCPUFlag(FLAG_SUBTRACT, false);
		cpu->SetCPUFlag(FLAG_HALF_CARRY, true);
	}

	void Instructions::BIT_HL(Cpu* cpu, uint8_t bit)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		BIT(cpu, value, bit);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::RES(uint8_t& reg, uint8_t bit)
	{
		reg &= ~(1 << bit);
	}

	void Instructions::RES_HL(Cpu* cpu, uint8_t bit)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		RES(value, bit);
		cpu->GetMMU().Write(state->HL, value);
	}

	void Instructions::SET(uint8_t& reg, uint8_t bit)
	{
		reg |= (1 << bit);
	}

	void Instructions::SET_HL(Cpu* cpu, uint8_t bit)
	{
		Cpu::sCPUState* state = cpu->GetState();

		uint8_t value = cpu->GetMMU().Read(state->HL);
		SET(value, bit);
		cpu->GetMMU().Write(state->HL, value);
	}
}