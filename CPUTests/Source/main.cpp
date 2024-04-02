
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
	Final final;
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

		file.close();
	}
}

bool RunTests(Core::Cpu cpu, std::vector<Test> tests)
{
	bool didPass = true;
	TestMmu& mmu = dynamic_cast<TestMmu&>(cpu.GetMMU());

	for (int i = 0; i < tests.size(); i++)
	{
		// reset devices
		cpu.Reset(false);
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
		cpuState.IME = tests[i].initial.IME;
		

		cpu.SetState(cpuState);

		// setup mmu state
		for (auto mem : tests[i].initial.ram)
		{
			mmu.Write(mem.address, mem.value, true);
		}

		// run CPU
		cpu.m_InstructionCompleted = false;
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
			std::cout << "Test: " << tests[i].name <<" State A - Result: " << static_cast<int>(state->A) << " Final: " << static_cast<int>(tests[i].final.A) << std::endl;
			didPass = false;
		}

		if (state->B != tests[i].final.B)
		{
			std::cout << "Test: " << tests[i].name <<" State B - Result: " << static_cast<int>(state->B) << " Final: " << static_cast<int>(tests[i].final.B) << std::endl;
			didPass = false;
		}

		if (state->C != tests[i].final.C)
		{
			std::cout << "Test: " << tests[i].name <<" State C - Result: " << static_cast<int>(state->C) << " Final: " << static_cast<int>(tests[i].final.C) << std::endl;
			didPass = false;
		}

		if (state->D != tests[i].final.D)
		{
			std::cout << "Test: " << tests[i].name <<" State D - Result: " << static_cast<int>(state->D) << " Final: " << static_cast<int>(tests[i].final.D) << std::endl;
			didPass = false;
		}

		if (state->E != tests[i].final.E)
		{
			std::cout << "Test: " << tests[i].name <<" State E - Result: " << static_cast<int>(state->E) << " Final: " << static_cast<int>(tests[i].final.E) << std::endl;
			didPass = false;
		}

		if (state->F != tests[i].final.F)
		{
			std::cout << "Test: " << tests[i].name <<" State F - Result: " << static_cast<int>(state->F) << " Final: " << static_cast<int>(tests[i].final.F) << std::endl;
			didPass = false;
		}

		if (state->H != tests[i].final.H)
		{
			std::cout << "Test: " << tests[i].name << " State H - Result: " << static_cast<int>(state->H) << " Final: " << static_cast<int>(tests[i].final.H) << std::endl;
			didPass = false;
		}

		if (state->L != tests[i].final.L)
		{
			std::cout << "Test: " << tests[i].name <<" State L - Result: " << static_cast<int>(state->L) << " Final: " << static_cast<int>(tests[i].final.L) << std::endl;
			didPass = false;
		}

		if (state->PC != tests[i].final.PC)
		{
			std::cout << "Test: " << tests[i].name <<" State PC - Result: " << static_cast<int>(state->PC) << " Final: " << static_cast<int>(tests[i].final.PC) << std::endl;
			didPass = false;
		}

		if (state->SP != tests[i].final.SP)
		{
			std::cout << "Test: " << tests[i].name <<" State SP - Result: " << static_cast<int>(state->SP) << " Final: " << static_cast<int>(tests[i].final.SP) << std::endl;
			didPass = false;
		}

		if (state->IME != tests[i].final.IME)
		{
			std::cout << "Test: " << tests[i].name <<" State IME - Result: " << static_cast<int>(state->IME) << " Final: " << static_cast<int>(tests[i].final.IME) << std::endl;
			didPass = false;
		}

		for (auto mem : tests[i].final.ram)
		{
			uint8_t result = mmu.Read(mem.address, true);
			if (result != mem.value)
			{
				std::cout << "Test: " << tests[i].name <<" RAM - Result: " << static_cast<int>(result) << " Final: " << static_cast<int>(mem.value) << std::endl;
				didPass = false;
			}
		}
	}

	return didPass;
}

int main()
{
	TestMmu mmu;
	Core::Cpu cpu(mmu);

	std::string path = "Source/Tests/";
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		std::cout << "Loading Test " << entry.path() << std::endl;

		std::vector<Test> tests = ParseTestsFromJSON(entry.path().string());
		bool didPass = RunTests(cpu, tests);

		if (didPass)
		{
			std::cout << "Test Passed" << std::endl;
		}
		else
		{
			std::cout << "********************** TEST FAILED **********************" << std::endl;
		}
	}
		
}