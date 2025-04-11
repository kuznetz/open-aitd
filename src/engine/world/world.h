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
		GameObject* followTarget = 0;

		World(Resources* res) {
			this->resources = res;
		}

		void setCurRoom(int stageId, int roomId) {
			if (curStageId != stageId) {
				curStageId = stageId;
				curStage = &resources->stages[stageId];
				curRoomId = -1;
				curCameraId = -1;
			}
			if (curRoomId != roomId) {
				curRoomId = roomId;
				curRoom = &curStage->rooms[roomId];
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
				auto& mdl = gobjects[i].animation;
				auto& mdlJson = objsJson[i]["model"];
				gobjects[i].modelId = mdlJson["id"];
				gobjects[i].boundsType = mdlJson["boundsType"];
				mdl.id = mdlJson["animId"];
				mdl.flags = mdlJson["animType"];
				mdl.nextId = mdlJson["animInfo"];
			}

			if (objsJson[i].contains("track")) {
				gobjects[i].track.id = objsJson[i]["track"]["id"];
				gobjects[i].track.mode = objsJson[i]["track"]["mode"];
				gobjects[i].track.pos = objsJson[i]["track"]["position"];
			}

			if (objsJson[i].contains("static")) {
				gobjects[i].location.stageId = objsJson[i]["static"]["stageId"];
				gobjects[i].location.roomId = objsJson[i]["static"]["roomId"];
				gobjects[i].staticColliderId = objsJson[i]["static"]["staticIdx"];
				auto& cols = resources->stages[gobjects[i].location.stageId].rooms[gobjects[i].location.roomId].colliders;
				for (int j = 0; j < cols.size(); j++) {
					if (cols[j].type != 9) continue;
					if (cols[j].parameter == gobjects[i].staticColliderId) {
						gobjects[i].staticCollider = &cols[j];
						break;
					}
				}
				auto& colB = gobjects[i].staticCollider->bounds;
				if (gobjects[i].staticCollider) {
					gobjects[i].location.position = {
						(colB.max.x - colB.min.x) / 2,
						(colB.max.y - colB.min.y) / 2,
						(colB.max.z - colB.min.z) / 2,
					};
				}
			}

			gobjects[i].lifeMode = objsJson[i]["lifeMode"];			
			gobjects[i].lifeId = objsJson[i]["life"];
		}
	};

	void World::setRepeatAnimation(GameObject& gobj, int animId) {
		gobj.animation.scriptAnimId = animId;
		gobj.animation.id = animId;
		gobj.animation.nextId = -1;
		gobj.animation.animTime = 0;
		gobj.animation.animEnd = 0;
		gobj.animation.flags = 1;
	}

	void World::setOnceAnimation(GameObject& gobj, int animId, int nextAnimId) {
		gobj.animation.scriptAnimId = animId;
		gobj.animation.id = animId;
		gobj.animation.nextId = nextAnimId;
		gobj.animation.animTime = 0;
		gobj.animation.animEnd = 0;
		gobj.animation.flags = 0;
	}

	void World::setModel(GameObject& gobj, int modelId) {
		gobj.modelId = modelId;
		gobj.animation.id = 0;
		gobj.animation.animTime = 0;
	}

	void World::loadVars(string path)
	{

	};

}