#pragma once
#include <vector>
#include <map>
#include <string>

#include "../raylib-cpp.h"
#include "bounds.h"
#include "config.h"
#include "model.h"

using namespace std;
namespace openAITD {

	struct RModel
	{
		Model model;
		map<int, int> animsIds;
		Bounds bounds;
	};

	class RModels
	{
	private:
		map<int, RModel> models;
		//For AITD1 - Emily character
		map<int, RModel> altModels;
	public:
		Config* config = 0;
		string modelsPath = "data/models";
		RModels();
		~RModels();
		RModel* getModel(int idx, bool alt = false);
		void clear();
	};

	RModels::RModels() {
	}

	RModel* RModels::getModel(int id, bool alt)
	{
		auto& modMap = alt ? models : altModels;
		auto mi = modMap.find(id);
		if (mi != modMap.end()) {
			return &(*mi).second;
		}
		string str = modelsPath + "/" + to_string(id) + (alt ? "_alt" : "") + "/model.gltf";
		auto& newMod = modMap[id];
		newMod.model.load(str.c_str());
		newMod.model.bakePoses(config->targetFps);

		if (newMod.model.data) {
			for (int i = 0; i < newMod.model.data->animations_count; i++) {
				char* s = newMod.model.data->animations[i].name + 2;
				int aNum = std::stoi(s);
				newMod.animsIds[aNum] = i;
			}
		}

		str = modelsPath + "/" + to_string(id) + (alt ? "_alt" : "") + "/data.json";
		std::ifstream ifs(str);
		json dataJson = json::parse(ifs);
		auto& b = dataJson["bounds"];

		newMod.bounds = { { b[0], b[1], b[2] }, { b[3], b[4], b[5] } };

		return &newMod;
	}

	void RModels::clear() {
		models.clear();
		altModels.clear();
	}

	RModels::~RModels() {
		clear();
	}

}