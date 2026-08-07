#pragma once
#include <vector>
#include <string>
#include "../../common/raylib_cpp.hpp"
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
		bool allowInventory;

		bool space;
		int keyboard;
	};

	struct Picture {
		int id = -1;
		float curTime;
		float delay;
	};

	struct BookData {
		int bookType;
		int readText = -1;
		int readAudio = -1;
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
		Picture picture;
		
		float chrono = 0;
		//Time, how long the room was active
		float roomChrono = 0;

		bool altModels = false;
		bool altBackgrounds = false;

		bool gameOver = true;
		bool inDark = false;
		
		int curStageId  = -1;
		int nextStageId = -1;
		Stage* curStage = 0;

		int curRoomId = -1;
		int nextRoomId = -1;
		Room* curRoom = 0;
		
		int curCameraId = -1;
		//Object to follow camera
		GameObject* followTarget = 0;

		string messageText;
		float messageTime = 0;

		GameObject* takedObj = 0;
		GameObject* curInvGObject = 0;
		GameObject* inHandObj = 0;
		int curInvAction = 0;

		BookData bookData;

		Vector3 debugShootFrom;
		Vector3 debugShootTo;

    float brightnessCur = 1;
    float brightnessTrg = 1;

		World(Resources* res) {
			this->resources = res;
		}

		bool isObjectActive(const GameObject& gobj) {
			if (gobj.lifeId == -1) return false;
			if (gobj.getStageId() != this->curStageId) return false;
			if (gobj.lifeMode == GOLifeMode::none) return false;
			if (gobj.lifeMode == GOLifeMode::room && gobj.getRoomId() != this->curRoomId) return false;
			if (gobj.lifeMode == GOLifeMode::roomInCamera) {
				if (this->curCameraId == -1) return false;				
				bool inCamera = false;
				auto& cam = this->curStage->cameras[this->curCameraId];
				for (int i = 0; i < cam.rooms.size(); i++) {
					if (gobj.getRoomId() == cam.rooms[i].roomId) {
						inCamera = true;
						break;
					}
				}
				if (!inCamera) return false;
			}
			return true;
		}

		void setCurStage(int stageId, int roomId) {
			nextStageId = stageId;
			nextRoomId = roomId;

			//if (stageId == -1) throw new exception("Stage = -1");
			//if (curStageId != stageId) {
			//	curStageId = stageId;
			//	curStage = (stageId != -1) ? &resources->stages[stageId] : 0;
			//	curRoomId = -1;
			//	curCameraId = -1;
			//}
			//setCurRoom(roomId);
		}


		void setCurRoom(int roomId) {
			if (curRoomId != roomId) {
				curRoomId = roomId;
				curRoom = (roomId != -1) ? &curStage->rooms[roomId]: 0;
				roomChrono = chrono;
			}
		}

		void setAltModels(bool newValue) {
			altModels = newValue;
			GameObject::altModels = newValue;
		};

		void preload() {
			//Preload Cameras
			resources->backgrounds.loadStage(curStageId);
							
			//Preload Objects
			for (int i = 0; i < gobjects.size(); i++) {
					auto& gobj = gobjects[i];
					if (gobj.modelId == -1) continue;
					if (gobj.getStageId() != curStageId) continue;
					resources->models.getModel(gobj.modelId, altModels);
			}
		}

		void loadGObjects(string path);
		void loadVars(string path);
		void setCharacter(bool alt) {}
		void setRepeatAnimation(GameObject& gobj, int animId);
		void setOnceAnimation(GameObject& gobj, int animId, int nextAnimId, bool uninterrupable = false);
		void setModel(GameObject& gobj, int modelId);
		void delFromInventory(int itemObjId);
		void take(int gObjId);
		void drop(int itemObjId, int actorObjId);
		void put(int objId, int room, int stage, Vector3 pos, EulerAngles rot);
		
		Vector3 AbsolutePos(const GameObject& gobj);
	};

	void World::loadGObjects(string path)
	{
		ifstream ifs(path);
		json objsJson = json::parse(ifs);

		gobjects.clear();
		gobjects.reserve(objsJson.size());
		for (int i=0; i<objsJson.size(); i++) {
				gobjects.emplace_back(*resources);
		}

		for (int i = 0; i < objsJson.size(); i++) {
			auto& gobj = gobjects[i];
			gobj.id = i;

			gobj.flags = objsJson[i]["flags"];
			gobj.lifeMode = objsJson[i]["lifeMode"];
			gobj.lifeId = objsJson[i]["life"];
			gobj.stageLifeId = objsJson[i]["stageLife"];
			gobj.chrono = chrono;

			if (objsJson[i].contains("name")) {
				gobj.name = objsJson[i]["name"];
			}

			if (objsJson[i].contains("location")) {
				auto& locJson = objsJson[i]["location"];
				gobj.setOrigRotation({ locJson["rotation"][0], locJson["rotation"][1], locJson["rotation"][2] });
				Vector3 pos = { locJson["position"][0], locJson["position"][1], locJson["position"][2] };
				gobj.setStage(locJson["stageId"], locJson["roomId"], pos);
			}

			if (objsJson[i].contains("model")) {
				auto& anim = gobj.animation;
				auto& mdlJson = objsJson[i]["model"];
				gobj.modelId = mdlJson["id"];
				gobj.boundsType = mdlJson["boundsType"];
				anim.id = mdlJson["animId"];
				anim.flags = mdlJson["animType"];
				if (anim.flags == 1) {
					gobj.bitField.animated = true;
				}
				anim.nextId = mdlJson["animInfo"];
			}

			if (objsJson[i].contains("track")) {
				gobj.track.id = objsJson[i]["track"]["id"];
				gobj.track.mode = objsJson[i]["track"]["mode"];
				gobj.track.pos = objsJson[i]["track"]["position"];
			}

			if (objsJson[i].contains("static")) {
				gobj.setStage(
					objsJson[i]["static"]["stageId"],
					objsJson[i]["static"]["roomId"],
					{}
				);
				gobj.staticColliderId = objsJson[i]["static"]["staticIdx"];
				auto& cols = resources->stages[gobj.getStageId()].rooms[gobj.getRoomId()].colliders;
				for (int j = 0; j < cols.size(); j++) {
					if (cols[j].type != 9) continue;
					if (cols[j].parameter == gobj.staticColliderId) {
						gobj.staticCollider = &cols[j];
						cols[j].linkedObjectId = i;
						break;
					}
				}
				auto& colB = gobj.staticCollider->bounds;
				if (gobj.staticCollider) {
					gobj.setPosition({
						(colB.max.x + colB.min.x) / 2,
						(colB.max.y + colB.min.y) / 2,
						(colB.max.z + colB.min.z) / 2,
					});
				}
			}

			if (objsJson[i].contains("invItem")) {
				gobj.invItem.nameId = objsJson[i]["invItem"]["name"];
				gobj.invItem.modelId = objsJson[i]["invItem"]["model"];
				gobj.invItem.lifeId = objsJson[i]["invItem"]["life"];
				gobj.invItem.flags = objsJson[i]["invItem"]["flags"];
			}
		}

	};

	Vector3 World::AbsolutePos(const GameObject& gobj) {
		auto& room = curStage->rooms[gobj.getRoomId()];
		const auto pos = gobj.getPosition();
		return Vector3Add(pos, room.origPosition);
	}

	void World::setRepeatAnimation(GameObject& gobj, int animId) {
		if (gobj.animation.bitField.uninterruptable && !gobj.animation.animEnd) return;
		if (gobj.animation.id == animId) return;
		cout << "setRepeatAnimation " << gobj.name << " " << animId << endl;
		if (animId == -1) {
			gobj.bitField.animated = 0;
			return;
		}
		gobj.animation.scriptAnimId = animId;
		gobj.animation.id = animId;
		gobj.animation.nextId = -1;
		gobj.animation.animEnd = 0;
		gobj.animation.flags = 0;
		gobj.animation.bitField.repeat = 1;
		gobj.animation.bitField.uninterruptable = false;
		gobj.bitField.animated = 1;
	}

	void World::setOnceAnimation(GameObject& gobj, int animId, int nextAnimId, bool uninterrupable) {
		if (gobj.animation.bitField.uninterruptable && !gobj.animation.animEnd) return;
		if (gobj.animation.id == animId) return;
	  cout << "setOnceAnimation " << gobj.name << " " << animId << " " << nextAnimId << endl;
		if (animId == -1) {
			gobj.bitField.animated = 0;
			return;
		}
		gobj.animation.scriptAnimId = animId;
		gobj.animation.id = animId;
		gobj.animation.nextId = nextAnimId;
		gobj.animation.animEnd = 0;
		gobj.animation.flags = 0;
		gobj.animation.bitField.uninterruptable = uninterrupable;
		gobj.bitField.animated = 1;
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
		gobj.setStage(-1, -1, {0,0,0});
		gobj.invItem.bitField.in_inventory = 1;
		gobj.bitField.foundable = 1;
		inventory.push_back(&gobj);
		takedObj = &gobj;
	};

	void World::delFromInventory(int itemObjId)
	{
		this->gobjects[itemObjId].invItem.bitField.in_inventory = 0;
		for (int i = 0; i < inventory.size(); i++) {
			if (inventory[i]->id == itemObjId) {
				inventory.erase(inventory.begin() + i);
				break;
			}
		}
	}

	void World::drop(int itemObjId, int actorObjId)
	{
		auto& item = this->gobjects[itemObjId];
		auto& actor = this->gobjects[actorObjId];

		delFromInventory(itemObjId);
		item.invItem.bitField.dropped = 1;
		item.bitField.foundable = 1;
		item.invItem.foundTimeout = chrono + 10;
		item.boundsType = BoundsType::rotated;
		item.setStage(actor.getStageId(), actor.getRoomId(), actor.getPosition());
		item.setOrigRotation(actor.getOrigRotation());
		//action?
	};

	void World::put(int itemObjId, int stage, int room, Vector3 pos, EulerAngles rot)
	{
		auto& item = this->gobjects[itemObjId];
		item.setStage(stage, room, pos);
		item.setOrigRotation(rot);
		delFromInventory(itemObjId);
		//item.bitField.foundable = 0;
	};

}