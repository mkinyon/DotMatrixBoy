
#pragma once

#include "json.hpp"

#include <deque>
#include <string>
#include <fstream>

using json = nlohmann::json;

namespace App
{
	struct sAppState
	{
		bool IsPaused = false;
		bool IsBootRomEnabled = false;

		bool ShowDebugger = false;
		bool ShowVRAMViewer = false;
		bool ShowLCD = true;
		bool ShowMemoryMap = false;
		bool ShowAudioDebugger = false;
		bool ShowConsole = false;

		std::deque<std::string> recentRoms;

		sAppState()
		{
			LoadStateFromFile();
		}

		void LoadStateFromFile()
		{
			std::ifstream file;
			file.open("Config.json");

			if (file.is_open())
			{
				json jState = json::parse(file);
				this->IsPaused = jState.at("IsPaused");
				this->IsBootRomEnabled = jState.at("IsBootRomEnabled");
				this->ShowDebugger = jState.at("ShowDebugger");
				this->ShowVRAMViewer = jState.at("ShowVRAMViewer");
				this->ShowLCD = jState.at("ShowLCD");
				this->ShowMemoryMap = jState.at("ShowMemoryMap");
				this->ShowAudioDebugger = jState.at("ShowAudioDebugger");
				this->ShowConsole = jState.at("ShowConsole");

				if (jState.contains("RecentRoms"))
				{
					json jRecentRoms = jState["RecentRoms"];

					for (const auto& rom : jRecentRoms)
					{
						recentRoms.push_back(rom);
					}
				}
			}
		}

		void SaveStateToFile() const
		{
			json jState;
			jState["IsPaused"] = this->IsPaused;
			jState["IsBootRomEnabled"] = this->IsBootRomEnabled;
			jState["ShowDebugger"] = this->ShowDebugger;
			jState["ShowVRAMViewer"] = this->ShowVRAMViewer;
			jState["ShowLCD"] = this->ShowLCD;
			jState["ShowMemoryMap"] = this->ShowMemoryMap;
			jState["ShowAudioDebugger"] = this ->ShowAudioDebugger;
			jState["ShowConsole"] = this->ShowConsole;

			json jRecentRoms;
			for (const auto& str : this->recentRoms)
			{
				jRecentRoms.push_back(str);
			}

			jState["RecentRoms"] = jRecentRoms;

			std::ofstream file("Config.json", std::ios::out);
			if (file.is_open()) 
			{
				std::string output = jState.dump();
				file.write(output.c_str(), output.size());
				file.close();
			}
		}

		void AddRecentRomEntry(std::string path)
		{
			// remove any previous entries of this path
			for (int i = 0; i < recentRoms.size(); i++)
			{
				if (recentRoms[i] == path)
				{
					recentRoms.erase(recentRoms.begin() + i);
				}
			}

			// add this entry to the top
			recentRoms.push_front(path);

			// trim the list to just the last 20 items
			while (recentRoms.size() > 20)
			{
				recentRoms.pop_back();
			}
		}
	};
}