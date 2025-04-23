#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>

#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>
using namespace nlohmann;

using namespace std;
namespace openAITD {

	struct MissedAnim {
		int model;
		vector<int> anims;
	};

	class MissedAnims {
	public:
		string path = "data/missed_anims.json";
		vector<MissedAnim> anims;

		MissedAnims() {
			load();
		}

		void addMissed(int modelId, int animId) {
			bool found = false;
			for (int i = 0; i < anims.size(); i++) {
				if (anims[i].model != modelId) continue;
				auto& v = anims[i].anims;
				if (std::find(v.begin(), v.end(), animId) != v.end()) {
					return;
				}
				found = true;
				anims[i].anims.push_back(animId);
			}
			if (!found) {
				anims.push_back({ modelId, {animId} });
			}

			printf("Miss animation %d in model %d\n", animId, modelId);
			save();
		}

		void load() {
			if (!std::filesystem::exists(path)) return;
			std::ifstream ifs(path);
			json objsJson = json::parse(ifs);
			for (int i = 0; i < objsJson.size(); i++) {
				MissedAnim m;
				m.model = objsJson[i]["model"];
				m.anims = objsJson[i]["anims"].get<std::vector<int>>();
			}
		};

		void save() {
			json outJson = json::array();
			for (int i = 0; i < anims.size(); i++) {
				json objJson = json::object();
				objJson["model"] = anims[i].model;
				objJson["anims"] = json::array();
				for (int j = 0; j < anims[i].anims.size(); j++) {
					objJson["anims"].push_back(anims[i].anims[j]);
				}
				outJson.push_back(objJson);
			}
			std::ofstream o(path);
			o << std::setw(2) << outJson << std::endl;
		};

	};

}