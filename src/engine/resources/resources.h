#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include "stages.h"
#include "tracks.h"
#include "models.h"
#include "../raylib.h"

using namespace std;
namespace openAITD {

	//Store static data in game
	class Resources {
	public:
		vector<Stage> stages;
		vector<Track> tracks;
		RModels models;

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