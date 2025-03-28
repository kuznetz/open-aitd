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
		map<int,ModelAnimation> animations;
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
		//TODO: animations

		return &newMod;
	}

	void RModels::clear() {
		for (const auto& kv : models) {
			UnloadModel(kv.second.model);
		}
		models.clear();
		for (const auto& kv : altModels) {
			UnloadModel(kv.second.model);
		}
		altModels.clear();
	}

	RModels::~RModels() {
		clear();
	}

}