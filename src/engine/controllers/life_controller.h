﻿#pragma once
#include "../resources/resources.h"
#include "../world/world.h"
#include <luacpp/luacpp.h>
#include <iostream>
#include <string>

using namespace std;
using namespace luacpp;

namespace openAITD {

	struct LifeFunc {
		bool executed;
		LuaFunction* func;
	};

	class LifeController {
	public:
		World* world;
		Resources* resources;
		LuaState* lua = 0;
		map <int, LifeFunc> funcs;
		std::function<bool(uint32_t, const LuaObject&)> execCb;
		Matrix roomMatrix;

		LifeController(Resources* res, World* world) {
			//roomMatrix = MatrixTranspose(MatrixMultiply(MatrixRotateZ(PI), MatrixRotateX(PI)));
			//roomMatrix = MatrixTranspose(MatrixMultiply(MatrixRotateY(PI), MatrixRotateX(PI)));
			//roomMatrix = MatrixTranspose(MatrixMultiply(MatrixRotateX(PI), MatrixRotateY(PI)));
			this->resources = res;
			this->world = world;
			initLua();
		}

		~LifeController() {
			delete lua;
		}

		void initExpressions() {
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].location.roomId;
				}, "ROOM");
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].modelId;
				}, "MODEL");
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].physics.collidedBy;
				}, "COL_BY");
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].physics.staticColl;
				}, "HARD_COLLIDER");		
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].physics.objectColl;
				}, "ACTOR_COLLIDER");
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].physics.zoneTriggered;
				}, "TRIGGER_COLLIDER");			

			lua->CreateFunction([this](int obj) -> int {
				return -1;
				}, "HIT_BY");			
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].animation.scriptAnimId;
				}, "ANIM");
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].animation.animEnd;
				}, "END_ANIM");
			lua->CreateFunction([this](int obj) -> int {
				//TODO: POSREL
				return 2;
				}, "POSREL");
			lua->CreateFunction([this](int obj) -> int {
				int r = (this->world->gobjects[obj].invItem.flags & 0xC000) ? 1 : 0;
				return r;
				}, "IS_FOUND");
			lua->CreateFunction([this](int obj) -> int {
				return (this->world->gobjects[obj].track.mark);
				}, "MARK");
			lua->CreateFunction([this](int obj) -> int {
				return (this->world->gobjects[obj].track.id);
				}, "NUM_TRACK");

			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].rotateAnim.lifeAngles[0];
				}, "ALPHA");
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].rotateAnim.lifeAngles[1];
				}, "BETA");
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].rotateAnim.lifeAngles[2];
				}, "GAMMA");

			lua->CreateFunction([this](int obj) -> int {
				return (int)((this->world->chrono - this->world->gobjects[obj].chrono) / 60.);
				}, "CHRONO");
			lua->CreateFunction([this](int obj) -> int {
				return (int)((this->world->chrono - this->world->roomChrono) / 60.);
				}, "ROOM_CHRONO");			
		}

		void initInstructions() {
			lua->CreateFunction([this](const char* message) {
				cout << "LUA: " << message << endl;
			}, "LOG");
			
			lua->CreateFunction([this](int obj) {
				//
			}, "MESSAGE");

			//Basic
			lua->CreateFunction([this](int obj, int modelId) {
				this->world->setModel(this->world->gobjects[obj], modelId);
			}, "SET_MODEL");
			lua->CreateFunction([this](int obj, int lifeId) {
				this->world->gobjects[obj].lifeId = lifeId;
			}, "SET_LIFE");
			lua->CreateFunction([this](int obj) {
				this->world->gobjects[obj].chrono = this->world->chrono;
			}, "START_CHRONO");
			lua->CreateFunction([this](int obj, int coll) {
				this->world->gobjects[obj].bitField.coll = coll;
			}, "TEST_COL");
			//Set object angle
			lua->CreateFunction([this](int obj, int x, int y, int z) {

			}, "SET_ANGLE");
			lua->CreateFunction([this](int obj) {
				//
			}, "SET_FLAGS");
			lua->CreateFunction([this](int obj) {
				auto& gobj = this->world->gobjects[obj];
				gobj.location.roomId = -1;
				gobj.location.stageId = -1;
				//ListWorldObjets[lifeTempVar1].flags2 |= 0x4000;
			}, "DELETE_OBJ");
			lua->CreateFunction([this](int obj) {
				this->world->followTarget = &this->world->gobjects[obj];
			}, "SET_CAMERA_TARGET");
			lua->CreateFunction([this](int obj, int stage, int room, int x, int y, int z) {
				auto& gobj = this->world->gobjects[obj];
				gobj.location.stageId = stage;
				gobj.location.roomId = room;
				gobj.location.position.x = x/1000.;
				gobj.location.position.y = -y / 1000.;
				gobj.location.position.z = -z / 1000.;
			}, "CHANGE_ROOM");


			//INVENTORY
			lua->CreateFunction([this](int obj) {
				}, "FOUND");
			lua->CreateFunction([this](int obj) {
				//TODO: IN_HAND
				}, "IN_HAND");
			lua->CreateFunction([this](int obj) {
				//TODO: TAKE
				}, "TAKE");

			//Animations, tracks, rotations
			lua->CreateFunction([this](int obj, int animId, int nextAnimId) {
				this->world->setOnceAnimation(this->world->gobjects[obj], animId, nextAnimId);
				}, "SET_ANIM_ONCE");

			lua->CreateFunction([this](int obj, int animId) {
				this->world->setRepeatAnimation(this->world->gobjects[obj], animId);
				}, "SET_ANIM_REPEAT");

			lua->CreateFunction([this](int obj, int trackMode, int trackId, int positionInTrack) {
				auto& gobj = this->world->gobjects[obj];
				gobj.track.mode = GOTrackMode(trackMode);
				gobj.track.id = trackId;
				gobj.track.pos = positionInTrack;
				gobj.track.posStarted = false;
				}, "SET_TRACKMODE");

			lua->CreateFunction([this](int obj, int toAngle, int time) {
				auto& gobj = this->world->gobjects[obj];
				gobj.rotateAnim.curTime = 0;
				gobj.rotateAnim.timeEnd = time / 60.;
				gobj.rotateAnim.from = gobj.location.rotation;
				auto rotTo = QuaternionFromAxisAngle({ 1,0,0 }, toAngle * 2 * PI / 1024.);
				gobj.rotateAnim.to = rotTo;
				gobj.rotateAnim.toLifeAngles[0] = toAngle;
				}, "SET_ALPHA");

			lua->CreateFunction([this](int obj, int toAngle, int time) {
				auto& gobj = this->world->gobjects[obj];
				if (gobj.rotateAnim.timeEnd > 0) return;
				toAngle += 512;
				gobj.rotateAnim.curTime = 0;
				gobj.rotateAnim.timeEnd = time / 60.;
				gobj.rotateAnim.from = gobj.location.rotation;
				auto rotTo = QuaternionFromAxisAngle({ 0,1,0 }, toAngle * 2. * PI / 1024.);
				gobj.rotateAnim.to = rotTo;
				gobj.rotateAnim.toLifeAngles[1] = toAngle;
				}, "SET_BETA");

			//Process track
			lua->CreateFunction([this](int obj) {
				this->world->gobjects[obj].moveFlag = true;
				}, "DO_MOVE");
			//Process rotation
			lua->CreateFunction([this](int obj) {
				this->world->gobjects[obj].moveFlag = true;
				}, "DO_ROT_ZV");
			//recalc bounds?
			lua->CreateFunction([this]() {
				//TODO: DEF_ZV
				}, "DEF_ZV");
			lua->CreateFunction([this]() {
				//TODO: DO_CARRE_ZV
				}, "DO_CARRE_ZV");
			lua->CreateFunction([this]() {
				//TODO: DO_REAL_ZV
				}, "DO_REAL_ZV");			 

			//Sound & music
			lua->CreateFunction([this](int obj, int sampleId, int animId, int animFrame) {
				//TODO: remember frame, not play in life
				if (this->world->gobjects[obj].animation.id != animId) return;
				if (this->world->gobjects[obj].animation.animTime != animFrame) return;
				//cout << "SET_ANIM_SOUND " << sampleId << endl;
				}, "SET_ANIM_SOUND");
			lua->CreateFunction([this](int sampleId) {
				//cout << "SOUND " << sampleId << endl;
				}, "SOUND");
			//Set Random sound frequency
			lua->CreateFunction([this](int freqDiff) {
				//TODO: RND_FREQ
				}, "RND_FREQ");
			lua->CreateFunction([this](int sampleId, int reserve) {
				//TODO: REP_SOUND
				}, "REP_SOUND");
			lua->CreateFunction([this](int sampleId, int nextSampleId) {
				//TODO: SOUND_THEN
				}, "SOUND_THEN");
			lua->CreateFunction([this](int musicId) {
				//TODO: SOUND_THEN
				}, "FADE_MUSIC");

			//SFX
			lua->CreateFunction([this](int water) {
				//TODO: SOUND_THEN
				}, "WATER");
			lua->CreateFunction([this](int shaking) {
				//TODO: SET_SHAKING
				}, "SET_SHAKING");
			lua->CreateFunction([this](int light) {
				//TODO: SET_LIGHT
				}, "SET_LIGHT");

			lua->CreateFunction([this](int light) {
				world->gameOver = true;
				}, "GAME_OVER");
		}

		void initLua() {
			lua = new LuaState(luaL_newstate(), true);
			execCb = ([](uint32_t x, const LuaObject& lobj) -> bool {
				cout << "out: " << lobj.ToInteger() << endl;
				return true;
			});

			initExpressions();
			initInstructions();

			//TODO: load vars

			string errstr;
			if (!lua->DoFile("data/scripts.lua", &errstr)) {
				cout << "Load life failed: " << errstr;
			}
			for (int i = 0; i < 1000; i++) {
				auto s = string("life_") + to_string(i);
				auto f = new LuaFunction(lua->GetFunction(s.c_str()));
				if (f->GetType() != LUA_TFUNCTION) continue;
				funcs[i].func = f;
			}
		}

		bool isObjectActive(GameObject& gobj) {
			if (gobj.id == 286) {
				int z = 0;
			}
			if (gobj.lifeId == -1) return false;
			if (gobj.location.stageId != world->curStageId) return false;
			if (gobj.lifeMode == GOLifeMode::none) return false;
			if (gobj.lifeMode == GOLifeMode::room && gobj.location.roomId != world->curRoomId) return false;
			if (gobj.lifeMode == GOLifeMode::roomInCamera) {
				bool inCamera = false;
				auto& cam = world->curStage->cameras[world->curCameraId];
				for (int i = 0; i < cam.rooms.size(); i++) {
					if (gobj.location.roomId == cam.rooms[i].roomId) {
						inCamera = true;
						break;
					}
				}
				if (!inCamera) return false;
			}
			return true;
		}

		void reloadVars() {
			for (int i = 0; i < world->vars.size(); i++) {
				string name = string("var_") + to_string(i);
				lua->CreateInteger(world->vars[i], name.c_str());
			}
		}

		void process() {
			
			for (auto it = funcs.begin(); it != funcs.end(); it++)
			{
				it->second.executed = false;
			}

			for (int i = 0; i < world->gobjects.size(); i++) {
				
				auto& gobj = world->gobjects[i];
				if (!isObjectActive(gobj)) continue;

				auto& f = funcs.find(gobj.lifeId);
				if (f == funcs.end()) {
					cout << "Life " << gobj.lifeId << "(obj: " << i << ") not found, " << endl;
					continue;
				}

				string errstr;
				if (!f->second.func->Execute(execCb, &errstr, i)) {
					cout << "Execute life_" << i << " error: " << errstr << endl;
				}
				f->second.executed = true;
			}

			/*string s = "";
			for (auto it = funcs.begin(); it != funcs.end(); it++)
			{
				if (it->second.executed) s += " " + to_string(it->first);
			}*/

		}

	};

}



