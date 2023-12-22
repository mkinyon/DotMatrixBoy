#include "Debugger.h"

namespace App
{
	Debugger::Debugger(Core::GameBoy& gb) : ImguiWindowBase("Debugger")
	{
		gameboy = gb;
	}

	Debugger::~Debugger() {}

	void Debugger::RenderContent()
	{
		// TODO: this syntax is stupid "gameboy.cpu.DisassebleAll(gameboy)"
		std::map<uint16_t, std::string> instructions = gameboy.cpu.DisassebleAll(gameboy);

		for (uint16_t i = 0; i < 0x7FFF; i++)
		{
			ImGui::Text(instructions[i].c_str());
		}
	}
}