#pragma once
#include <vector>
#include <map>
#include <string>

#include "../raylib.h"

using namespace std;
namespace openAITD {

	struct RModel
	{
		Model model;
		ModelAnimation* animationsPtr;
		map<int,ModelAnimation*> animations;
		int animCount;
		BoundingBox bounds;
	};

	class RModels
	{
	private:
		map<int, RModel> models;
		//For AITD1 - female character
		map<int, RModel> altModels;
	public:
		string modelsPath = "data/models";
		~RModels();
		RModel* getModel(int idx, bool alt = false);
		void clear();
	};

	RModel* RModels::getModel(int id, bool alt)
	{
		auto& modMap = alt ? models : altModels;
		auto mi = modMap.find(id);
		if (mi != modMap.end()) {
			return &(*mi).second;
		}
		string str = modelsPath + "/" + to_string(id) + (alt ? "_alt" : "") + "/model.gltf";
		auto& newMod = modMap[id];
		newMod.model = LoadModel(str.c_str());		
		newMod.animationsPtr = LoadModelAnimations(str.c_str(), &newMod.animCount);
		for (int i = 0; i < newMod.animCount; i++) {
			char* s = newMod.animationsPtr[i].name + 2;
			int aNum = std::stoi(s);
			newMod.animations[aNum] = &newMod.animationsPtr[i];
		}

		str = modelsPath + "/" + to_string(id) + (alt ? "_alt" : "") + "/data.json";
		std::ifstream ifs(str);
		json dataJson = json::parse(ifs);
		auto& b = dataJson["bounds"];

		newMod.bounds.min = { b[0], b[1], b[2] };
		newMod.bounds.max = { b[3], b[4], b[5] };

		return &newMod;
	}

	void RModels::clear() {
		for (const auto& kv : models) {
			UnloadModel(kv.second.model);
			if (kv.second.animCount) {
				UnloadModelAnimations(kv.second.animationsPtr, kv.second.animCount);
			}
		}
		models.clear();
		for (const auto& kv : altModels) {
			UnloadModel(kv.second.model);
			if (kv.second.animCount) {
				UnloadModelAnimations(kv.second.animationsPtr, kv.second.animCount);
			}
		}
		altModels.clear();
	}

	RModels::~RModels() {
		clear();
	}

}