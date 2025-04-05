#pragma once
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
				gobj.trackMode = trackMode;
				gobj.trackId = trackId;
				gobj.trackPos = positionInTrack;
			}, "SET_TRACKMODE");
			
			lua->CreateFunction([this](int obj) {
				//
			}, "SET_FLAGS");
			
			lua->CreateFunction([this](int obj, int soundId, int animId, int animFrame) {
				//TODO: remember frame, not play in life
				if (this->world->gobjects[obj].model.animId != animId) return;
				if (this->world->gobjects[obj].model.animTime != animFrame) return;
				cout << "SET_ANIM_SOUND " << soundId << endl;
			}, "SET_ANIM_SOUND");

			lua->CreateFunction([this](int soundId) {
				cout << "SOUND " << soundId << endl;
			}, "SOUND");			

			lua->CreateFunction([this](int obj) {
			}, "FOUND");

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
				auto& f = funcs.find(gobj.lifeId);
				if (f == funcs.end()) continue;

				string errstr;
				if (!f->second->Execute(execCb, &errstr, i)) {
					cout << "Execute life_" << i << " error: " << errstr;
				}
			}
		}

	};

}