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
using namespace raylib;
namespace openAITD {

	struct PlayerAnimations {
		int idle = 4;
		int walkForw = 254;
		int runForw = 255;
		int idle2 = 4;
		int walkBackw = 256;
		int turnCW = 257;
		int turnCCW = 258;
	};

	struct Player {
		int objectId;
		bool hasControl;
		PlayerAnimations animations;
		bool space;
		bool allowInventory;
	};

	//Store all dynamic data in game
	class World {
	public:
		Resources* resources;
		vector<string> dataDirectories = { "./mods", "./oaitd", "./data" };
		
		vector<GameObject> gobjects;
		vector<GameObject*> inventory;

		vector<short int> vars;
		vector<short int> cVars;
		Player player;
		
		float chrono = 0;
		//Time, how long the room was active
		float roomChrono = 0;

		bool gameOver = false;
		int curStageId  = -1;
		Stage* curStage = 0;
		int curRoomId = -1;
		Room* curRoom = 0;
		int curCameraId = -1;
		//Object to follow camera
		GameObject* followTarget = 0;

		int foundItem = -1;

		string messageText;
		float messageTime = 0;

		GameObject* takedObj = 0;
		GameObject* curInvGObject = 0;
		GameObject* inHandObj = 0;
		int curInvAction = 0;

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
				roomChrono = chrono;
			}
		}
		void loadGObjects(string path);
		void loadVars(string path);
		void setCharacter(bool alt) {}
		void setRepeatAnimation(GameObject& gobj, int animId);
		void setOnceAnimation(GameObject& gobj, int animId, int nextAnimId);
		void setModel(GameObject& gobj, int modelId);
		void take(int gObjId);
		Vector3 VectorChangeRoom(const Vector3 v, int fromRoomId, int toRoomId);
		BoundingBox BoundsChangeRoom(const BoundingBox v, int fromRoomId, int toRoomId);
	};

	void World::loadGObjects(string path)
	{
		ifstream ifs(path);
		json objsJson = json::parse(ifs);

		gobjects.resize(objsJson.size());
		for (int i = 0; i < objsJson.size(); i++) {
			auto& gobj = gobjects[i];
			gobj.id = i;

			if (objsJson[i].contains("location")) {
				auto& loc = gobj.location;
				auto& locJson = objsJson[i]["location"];
				loc.position = { locJson["position"][0], locJson["position"][1], locJson["position"][2] };
				loc.rotation = { locJson["rotation"][0], locJson["rotation"][1], locJson["rotation"][2], locJson["rotation"][3] };
				loc.stageId = locJson["stageId"];
			    loc.roomId = locJson["roomId"];
			}

			if (objsJson[i].contains("model")) {
				auto& mdl = gobj.animation;
				auto& mdlJson = objsJson[i]["model"];
				gobj.modelId = mdlJson["id"];
				gobj.boundsType = mdlJson["boundsType"];
				mdl.id = mdlJson["animId"];
				mdl.flags = mdlJson["animType"];
				mdl.nextId = mdlJson["animInfo"];
			}

			if (objsJson[i].contains("track")) {
				gobj.track.id = objsJson[i]["track"]["id"];
				gobj.track.mode = objsJson[i]["track"]["mode"];
				gobj.track.pos = objsJson[i]["track"]["position"];
			}

			if (objsJson[i].contains("static")) {
				gobj.location.stageId = objsJson[i]["static"]["stageId"];
				gobj.location.roomId = objsJson[i]["static"]["roomId"];
				gobj.staticColliderId = objsJson[i]["static"]["staticIdx"];
				auto& cols = resources->stages[gobj.location.stageId].rooms[gobj.location.roomId].colliders;
				for (int j = 0; j < cols.size(); j++) {
					if (cols[j].type != 9) continue;
					if (cols[j].parameter == gobj.staticColliderId) {
						gobj.staticCollider = &cols[j];
						break;
					}
				}
				auto& colB = gobj.staticCollider->bounds;
				if (gobj.staticCollider) {
					gobj.location.position = {
						(colB.max.x + colB.min.x) / 2,
						(colB.max.y + colB.min.y) / 2,
						(colB.max.z + colB.min.z) / 2,
					};
				}
			}

			if (objsJson[i].contains("invItem")) {
				gobj.invItem.nameId = objsJson[i]["invItem"]["name"];
				gobj.invItem.modelId = objsJson[i]["invItem"]["model"];
				gobj.invItem.lifeId = objsJson[i]["invItem"]["life"];
				gobj.invItem.flags = objsJson[i]["invItem"]["flags"];
			}

			gobj.flags = objsJson[i]["flags"];
			gobj.lifeMode = objsJson[i]["lifeMode"];
			gobj.lifeId = objsJson[i]["life"];
			gobj.stageLifeId = objsJson[i]["stageLife"];
			gobj.chrono = chrono;
		}

	};

	Vector3 World::VectorChangeRoom(const Vector3 v, int fromRoomId, int toRoomId) {
		if (fromRoomId == toRoomId) return v;
		auto& roomFrom = curStage->rooms[fromRoomId];
		auto& roomTo   = curStage->rooms[toRoomId];
		return Vector3Subtract( Vector3Add(v, roomFrom.position), roomTo.position);
	}

	BoundingBox World::BoundsChangeRoom(const BoundingBox b, int fromRoomId, int toRoomId) {
		if (fromRoomId == toRoomId) return b;
		auto& roomFrom = curStage->rooms[fromRoomId].position;
		auto& roomTo = curStage->rooms[toRoomId].position;
		return {
			Vector3Subtract(Vector3Add(b.min, roomFrom), roomTo),
			Vector3Subtract(Vector3Add(b.max, roomFrom), roomTo)
		};
	}

	void World::setRepeatAnimation(GameObject& gobj, int animId) {
		gobj.animation.scriptAnimId = animId;
		gobj.animation.id = animId;
		gobj.animation.nextId = -1;
		gobj.animation.flags = 1;
	}

	void World::setOnceAnimation(GameObject& gobj, int animId, int nextAnimId) {
		gobj.animation.scriptAnimId = animId;
		gobj.animation.id = animId;
		gobj.animation.nextId = nextAnimId;
		gobj.animation.flags = 0;
	}

	void World::setModel(GameObject& gobj, int modelId) {
		gobj.modelId = modelId;
		//gobj.animation.id = 0;
		//gobj.animation.animTime = 0;
	}

	void World::loadVars(string path)
	{
		ifstream ifs(path);
		json objsJson = json::parse(ifs);
		vars.resize(objsJson["vars"].size());
		for (int i = 0; i < objsJson["vars"].size(); i++) {
			vars[i] = objsJson["vars"][i];
		}
		cVars.resize(objsJson["cVars"].size());
		for (int i = 0; i < objsJson["cVars"].size(); i++) {
			cVars[i] = objsJson["cVars"][i];
		}
	};

	void World::take(int gobjId)
	{
		auto& gobj = this->gobjects[gobjId];
		gobj.location.stageId = -1;
		gobj.invItem.bitField.in_inventory = 1;
		inventory.push_back(&gobj);
		takedObj = &gobj;
	};

}