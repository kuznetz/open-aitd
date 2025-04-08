﻿#pragma once
#include "../resources/resources.h"
#include "../world/world.h"
#include <luacpp/luacpp.h>
#include <iostream>
#include <string>

using namespace std;
using namespace luacpp;

namespace openAITD {

	class LifeController {
	public:
		World* world;
		Resources* resources;
		LuaState* lua = 0;
		map <int,LuaFunction*> funcs;
		std::function<bool(uint32_t, const LuaObject&)> execCb;

		LifeController(Resources* res, World* world) {
			this->resources = res;
			this->world = world;
			initLua();
		}

		~LifeController() {
			delete lua;
		}

		void initExpressions() {
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].model.id;
			}, "MODEL");
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].physics.collidedBy;
			}, "COL_BY");
			lua->CreateFunction([this](int obj) -> int {
				return -1;
			}, "HIT_BY");			
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].model.scriptAnimId;
			}, "ANIM");
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].model.animEnd;
			}, "END_ANIM");
			lua->CreateFunction([this](int obj) -> int {
				//TODO: realize
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
		}

		void initInstructions() {
			lua->CreateFunction([this](const char* message) {
				cout << "LUA: " << message << endl;
			}, "LOG");
			
			lua->CreateFunction([this](int obj) {
				//
			}, "MESSAGE");
			
			lua->CreateFunction([this](int obj, int modelId) {
				this->world->setModel(this->world->gobjects[obj], modelId);
			}, "SET_MODEL");

			lua->CreateFunction([this](int obj, int lifeId) {
				this->world->gobjects[obj].lifeId = lifeId;
			}, "SET_LIFE");			
			
			lua->CreateFunction([this](int obj, int animId, int nextAnimId) {
				this->world->setOnceAnimation(this->world->gobjects[obj], animId, nextAnimId);
			}, "SET_ANIM_ONCE");
			
			lua->CreateFunction([this](int obj, int trackMode, int trackId, int positionInTrack) {
				auto& gobj = this->world->gobjects[obj];
				gobj.track.mode = GOTrackMode(trackMode);
				gobj.track.id = trackId;
				gobj.track.pos = positionInTrack;
			}, "SET_TRACKMODE");
			
			lua->CreateFunction([this](int obj) {
				//
			}, "SET_FLAGS");
			
			lua->CreateFunction([this](int obj, int sampleId, int animId, int animFrame) {
				//TODO: remember frame, not play in life
				if (this->world->gobjects[obj].model.animId != animId) return;
				if (this->world->gobjects[obj].model.animTime != animFrame) return;
				//cout << "SET_ANIM_SOUND " << sampleId << endl;
			}, "SET_ANIM_SOUND");
			lua->CreateFunction([this](int sampleId) {
				//cout << "SOUND " << sampleId << endl;
			}, "SOUND");			
			lua->CreateFunction([this](int freq) {
				//TODO: Random sound frequency
			}, "RND_FREQ");
			lua->CreateFunction([this](int sampleId, int reserve) {
				//TODO: REP_SOUND
			}, "REP_SOUND");
			lua->CreateFunction([this](int sampleId, int nextSampleId) {
				//TODO: SOUND_THEN
			}, "SOUND_THEN");
			

			lua->CreateFunction([this](int obj) {
			}, "FOUND");

			lua->CreateFunction([this](int obj) {
				this->world->gobjects[obj].moveFlag = true;
			}, "DO_MOVE");

			lua->CreateFunction([this](int obj) {
				this->world->followTarget = &this->world->gobjects[obj];
			}, "SET_CAMERA_TARGET");
			lua->CreateFunction([this](int obj, int stage, int room, int x, int y, int z) {
				auto& gobj = this->world->gobjects[obj];
				gobj.location.stageId = stage;
				gobj.location.roomId = room;
				gobj.location.position.x = x/1000.;
				gobj.location.position.y = y / 1000.;
				gobj.location.position.z = z / 1000.;
			}, "CHANGE_ROOM");
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
				funcs[i] = f;
			}
		}

		void process() {
			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj = world->gobjects[i];
				if (gobj.lifeId == -1) continue;
				if (gobj.location.stageId != world->curStageId) continue;
				//if (gobj.lifeMode == GOLifeMode::none || gobj.lifeMode == GOLifeMode::off) continue;
				//if ((gobj.lifeMode == GOLifeMode::room || gobj.lifeMode == GOLifeMode::camera) && (gobj.location.roomId != world->curRoomId)) continue;

				auto& f = funcs.find(gobj.lifeId);
				if (f == funcs.end()) {
					cout << "Life " << gobj.lifeId << "(obj: " << i << ") not found, " << endl;
					continue;
				}

				string errstr;
				if (!f->second->Execute(execCb, &errstr, i)) {
					cout << "Execute life_" << i << " error: " << errstr << endl;
				}
			}
		}

	};

}