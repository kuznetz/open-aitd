﻿#pragma once
#include <vector>
#include <map>
#include <string>
#include <filesystem>
#include "config.h"
#include "stages.h"
#include "tracks.h"
#include "models.h"
#include "../raylib.h"

using namespace std;
namespace openAITD {

	//Store static data in game
	class Resources {
	public:
		Config config;
		Font mainFont;
		map<int,string> texts;

		vector<Stage> stages;
		vector<Track> tracks;
		RModels models;

		~Resources() {
			UnloadFont(mainFont);
		}

		void loadFont(string fontPath, int size) {
			//Font fontTtf = LoadFontEx
			//FONT_TTF_DEFAULT_NUMCHARS 95
			mainFont = LoadFontEx(fontPath.c_str(), size, 0, 95);
		}

		void loadTexts(string textsPath) {
			int idx;
			string str;
			ifstream inFile;
			inFile.open(textsPath);
			while (getline(inFile, str))
			{
				if (str[0] != '@') continue;
				idx = 0;
				int i = 1;
				while (str[i] >= '0' && str[i] <= '9') // parse string number
				{
					idx = idx * 10 + (str[i] - 48);
					i++;
				}
				if (str[i] == ':') // start of string
				{
					texts[idx] = str.substr(i+1);
				}
			}
		}


		void loadTracks(string path) {
			int i = 0;
			while (true) {
				string s = path + "/" + to_string(i) + ".json";
				if (!std::filesystem::exists(s)) break;
				std::ifstream ifs(s);
				json objsJson = json::parse(ifs);

				auto& track = tracks.emplace_back();
				for (int j = 0; j < objsJson.size(); j++ ) {
					auto& t = track.emplace_back();
					t.type = objsJson[j]["type"];
					if (objsJson[j].contains("room")) {
						t.room = objsJson[j]["room"];
					}
					if (objsJson[j].contains("mark")) {
						t.mark = objsJson[j]["mark"];
					}
					if (objsJson[j].contains("time")) {
						t.time = objsJson[j]["time"];
					}
					if (objsJson[j].contains("pos")) {
						t.pos.x = objsJson[j]["pos"][0];
						t.pos.y = objsJson[j]["pos"][1];
						t.pos.z = objsJson[j]["pos"][2];
					}
					if (objsJson[j].contains("rot")) {
						t.rot.x = objsJson[j]["rot"][0];
						t.rot.y = objsJson[j]["rot"][1];
						t.rot.z = objsJson[j]["rot"][2];
					}
				}

				i++;
			}
		};

	};

}