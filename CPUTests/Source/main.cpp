
#include "json.hpp"
#include "Core/Cpu.h"
#include "TestMmu.h"

#include <fstream>
#include <vector>

using json = nlohmann::json;

struct CycleEntry
{
	uint16_t address;
	uint8_t value;
	std::string description;
};

struct RamEntry
{
	uint16_t address = 0;
	uint8_t value = 0;
};

struct Initial
{
	uint16_t PC = 0;
	uint16_t SP = 0;
	uint8_t A = 0;
	uint8_t B = 0;
	uint8_t C = 0;
	uint8_t D = 0;
	uint8_t E = 0;
	uint8_t F = 0;
	uint8_t H = 0;
	uint8_t L = 0;
	uint8_t IME = 0;
	uint8_t IE = 0;
	std::vector<RamEntry> ram;
};

struct Final
{
	uint16_t PC = 0;
	uint16_t SP = 0;
	uint8_t A = 0;
	uint8_t B = 0;
	uint8_t C = 0;
	uint8_t D = 0;
	uint8_t E = 0;
	uint8_t F = 0;
	uint8_t H = 0;
	uint8_t L = 0;
	uint8_t IME = 0;
	std::vector<RamEntry> ram;
};

struct Test
{
	std::string name;
	Initial initial;
	Initial final;
	std::vector<CycleEntry> cycles;
};

std::vector<Test> ParseTestsFromJSON(std::string filePath)
{
	std::vector<Test> tests;

	std::ifstream file;
	file.open(filePath);

	if (file.is_open())
	{
		json data = json::parse(file);

		for (auto& jTestItem : data.items())
		{
			json jTest = jTestItem.value();

			// get name
			Test test;
			test.name = jTest.at("name");

			// get initial
			json jInitial = jTest.at("initial");
			test.initial.A = jInitial.at("a");
			test.initial.B = jInitial.at("b");
			test.initial.C = jInitial.at("c");
			test.initial.D = jInitial.at("d");
			test.initial.E = jInitial.at("e");
			test.initial.F = jInitial.at("f");
			test.initial.H = jInitial.at("h");
			test.initial.L = jInitial.at("l");
			test.initial.PC = jInitial.at("pc");
			test.initial.SP = jInitial.at("sp");
			test.initial.IE = jInitial.at("ie");
			test.initial.IME = jInitial.at("ime");

			// get initial ram
			for (auto& jRamEntryItem : jInitial.at("ram"))
			{
				RamEntry ramEntry;
				ramEntry.address = jRamEntryItem[0];
				ramEntry.value = jRamEntryItem[1];

				test.initial.ram.push_back(ramEntry);
			}

			// get final
			json jFinal = jTest.at("final");
			test.final.A = jFinal.at("a");
			test.final.B = jFinal.at("b");
			test.final.C = jFinal.at("c");
			test.final.D = jFinal.at("d");
			test.final.E = jFinal.at("e");
			test.final.F = jFinal.at("f");
			test.final.H = jFinal.at("h");
			test.final.L = jFinal.at("l");
			test.final.PC = jFinal.at("pc");
			test.final.SP = jFinal.at("sp");
			test.final.IME = jFinal.at("ime");

			// get final ram
			for (auto& jRamEntryItem : jFinal.at("ram"))
			{
				RamEntry ramEntry;
				ramEntry.address = jRamEntryItem[0];
				ramEntry.value = jRamEntryItem[1];

				test.final.ram.push_back(ramEntry);
			}

			// get cycles
			for (auto& jCyclesEntry : jTest.at("cycles"))
			{
				CycleEntry cycleEntry;
				cycleEntry.address = jCyclesEntry[0];
				cycleEntry.value = jCyclesEntry[1];
				cycleEntry.description = jCyclesEntry[2];

				test.cycles.push_back(cycleEntry);
			}

			tests.push_back(test);
		}

		return tests;
	}
}

bool RunTests(Core::Cpu cpu, std::vector<Test> tests)
{
	TestMmu& mmu = dynamic_cast<TestMmu&>(cpu.GetMMU());

	for (int i = 0; i < tests.size(); i++)
	{
		// Reset Ram
		mmu.Reset();

		// setup CPU State
		Core::Cpu::sCPUState cpuState;

		cpuState.A = tests[i].initial.A;
		cpuState.B = tests[i].initial.B;
		cpuState.C = tests[i].initial.C;
		cpuState.D = tests[i].initial.D;
		cpuState.E = tests[i].initial.E;
		cpuState.F = tests[i].initial.F;
		cpuState.H = tests[i].initial.H;
		cpuState.L = tests[i].initial.L;
		cpuState.PC = tests[i].initial.PC;
		cpuState.SP = tests[i].initial.SP;

		cpu.SetState(cpuState);

		// setup mmu state
		for (auto mem : tests[i].initial.ram)
		{
			mmu.Write(mem.address, mem.value, true);
		}

		// run CPU
		for (auto cycle : tests[i].cycles)
		{
			while (!cpu.m_InstructionCompleted)
			{
				cpu.Clock();
			}
		}

		// verify/output results
		Core::Cpu::sCPUState* state = cpu.GetState();
		if (state->A != tests[i].final.A)
		{
			return false;
		}
	}

	return true;
}

int main()
{
	TestMmu mmu;
	Core::Cpu cpu(mmu);

	std::vector<Test> tests = ParseTestsFromJSON("Source/Tests/00.json");
	bool didPass = RunTests(cpu, tests);
}