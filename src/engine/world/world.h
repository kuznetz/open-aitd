#pragma once
#include <vector>
#include <string>
#include "../raylib.h"
#include "game_objects.h"

#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>
using nlohmann::json;

using namespace std;
namespace openAITD {

	struct Player {
		int objectId;
		bool hasControl;
		int animations[6];
	};

	//Store all dynamic data in game
	class World {
	public:
		vector<string> dataDirectories = { "./mods", "./oaitd", "./data" };
		vector<GameObject> gobjects;
		vector<short int> vars;
		vector<short int> cVars;
		//Object to follow camera
		int trackedGObjectId;
		int curCameraId;
		int curStageId;

		void loadGObjects(string path);
		void loadVars(string path);
		void setCharacter(bool alt) {}
	};

	void World::loadGObjects(string path)
	{
		std::ifstream ifs(path);
		json objsJson = json::parse(ifs);

		gobjects.resize(objsJson.size());
		for (int i = 0; i < objsJson.size(); i++) {
			if (objsJson[i].contains("location")) {
				auto& loc = gobjects[i].location;
				auto& locJson = objsJson[i]["location"];
				loc.position = { locJson["position"][0], locJson["position"][1], locJson["position"][2] };
				loc.rotation = { locJson["rotation"][0], locJson["rotation"][1], locJson["rotation"][2], locJson["rotation"][3] };
				loc.stageId = locJson["stageId"];
			    loc.roomId = locJson["roomId"];
			}

			if (objsJson[i].contains("model")) {
				auto& mdl = gobjects[i].model;
				auto& mdlJson = objsJson[i]["model"];
				mdl.id = mdlJson["id"];
				mdl.animId = mdlJson["animId"];
				mdl.animType = mdlJson["animType"];
				mdl.animInfo = mdlJson["animInfo"];
			}
		}
	};

	void World::loadVars(string path)
	{

	};

}