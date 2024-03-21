
#include "json.hpp"
#include "Core/Cpu.h"

#include <fstream>
#include <vector>

using json = nlohmann::json;

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
};

int main()
{
	std::vector<Test> tests;
	Core::Cpu cpu();

	std::ifstream file;
	file.open("Source/Tests/00.json");
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
			//for (auto& jRamEntryItem : jInitial.at("ram"))
			//{
			//	//json jRamEntry = jRamEntryItem;

			//	for (auto& jRamEntryItem2 : jRamEntryItem.items())
			//	{
			//		json test = jRamEntryItem2.value();

			//		RamEntry ramEntry;
			//		ramEntry.address = test.at("0");
			//		ramEntry.value = test.at("1");
			//	}
			//}

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

			tests.push_back(test);
		}

		auto results = tests;
	}
	
}