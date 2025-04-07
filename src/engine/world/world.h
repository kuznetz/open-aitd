#pragma once
#include <vector>
#include <string>
#include "../raylib.h"
#include "game_objects.h"
#include "../resources/resources.h"

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
		Resources* resources;
		vector<string> dataDirectories = { "./mods", "./oaitd", "./data" };
		vector<GameObject> gobjects;
		vector<short int> vars;
		vector<short int> cVars;
		
		int curStageId  = -1;
		Stage* curStage = 0;
		int curRoomId = -1;
		Room* curRoom = 0;
		int curCameraId = -1;
		//Object to follow camera
		GameObject* followTarget;

		World(Resources* res) {
			this->resources = res;
		}

		void setCurRoom(int stageId, int roomId) {
			curStageId = stageId;
			curStage = &resources->stages[stageId];
			curRoomId = roomId;
			curRoom = &curStage->rooms[roomId];
			if (curStageId != stageId) {
				curCameraId = -1;
			}
		}
		void loadGObjects(string path);
		void loadVars(string path);
		void setCharacter(bool alt) {}
		void setRepeatAnimation(GameObject& gobj, int animId);
		void setOnceAnimation(GameObject& gobj, int animId, int nextAnimId);
		void setModel(GameObject& gobj, int modelId);
	};

	void World::loadGObjects(string path)
	{
		std::ifstream ifs(path);
		json objsJson = json::parse(ifs);

		gobjects.resize(objsJson.size());
		for (int i = 0; i < objsJson.size(); i++) {
			gobjects[i].id = i;

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
				mdl.flags = mdlJson["animType"];
				mdl.nextAnimId = mdlJson["animInfo"];
				mdl.boundsType = mdlJson["boundsType"];
			}

			if (objsJson[i].contains("track")) {
				gobjects[i].track.id = objsJson[i]["track"]["id"];
				gobjects[i].track.mode = objsJson[i]["track"]["mode"];
				gobjects[i].track.pos = objsJson[i]["track"]["position"];
			}

			gobjects[i].lifeMode = objsJson[i]["lifeMode"];			
			gobjects[i].lifeId = objsJson[i]["life"];
		}
	};

	void World::setRepeatAnimation(GameObject& gobj, int animId) {
		gobj.model.scriptAnimId = animId;
		gobj.model.animId = animId;
		gobj.model.nextAnimId = -1;
		gobj.model.animTime = 0;
		gobj.model.animEnd = 0;
		gobj.model.flags = 1;
	}

	void World::setOnceAnimation(GameObject& gobj, int animId, int nextAnimId) {
		gobj.model.scriptAnimId = animId;
		gobj.model.animId = animId;
		gobj.model.nextAnimId = nextAnimId;
		gobj.model.animTime = 0;
		gobj.model.animEnd = 0;
		gobj.model.flags = 0;
	}

	void World::setModel(GameObject& gobj, int modelId) {
		gobj.model.id = modelId;
		gobj.model.animTime = 0;
	}

	void World::loadVars(string path)
	{

	};

}