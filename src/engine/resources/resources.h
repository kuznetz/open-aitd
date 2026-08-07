#pragma once
#include <vector>
#include <map>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include "config.h"
#include "stages.h"
#include "tracks.h"
#include "models.h"
#include "backgrounds.h"
#include "screen.h"
#include "audio.h"
#include "texts.h"
#include "../../common/raylib_cpp.hpp"
#include "../../common/name_decoders.hpp"

using namespace std;
namespace openAITD {

	//Store static data in game
	class Resources {
	public:
		Config config;
		NameDecoders nameDecoders;
		vector<Stage> stages;
		vector<Track> tracks;
		RModels models;
		Backgrounds backgrounds;
		Screen screen;
		Audio audio;
		Texts texts;

		Resources():
			screen(config),
			texts(config)
		{
			nameDecoders.load();
			backgrounds.loadAltBackgrounds();
			models.config = &config;
			backgrounds.config = &config;
			backgrounds.stages = &stages;
			models.nameDecoders = &nameDecoders;
		}

		~Resources() {
			printf("Deleting resources\n");
		}

		void loadTracks(string dataPath, string newDataPath) {
			int i = 0;
			while (true) {
				string s = newDataPath + "/" + to_string(i) + ".json";
				if (!std::filesystem::exists(s)) {
					s = dataPath + "/" + to_string(i) + ".json";
					if (!std::filesystem::exists(s)) break;
				}
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

		bool isRoomsConnected(const Stage& stage, int room1, int room2) {
			const Room& room = stage.rooms[room1];
			for (int j = 0; j < room.zones.size(); j++) {
				if (room.zones[j].type != RoomZoneType::ChangeRoom) continue;
				if (room2 == room.zones[j].parameter) {
					return true;
				}
			}
			return false;
		}

		void setLanguage(const string lang) {
			texts.setLanguage(lang);
		}

	};

}