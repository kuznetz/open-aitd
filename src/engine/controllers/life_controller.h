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
				//TODO: realize
				return 0;
			}, "COL_BY");
			lua->CreateFunction([this](int obj) -> int {
				//TODO: realize
				return this->world->gobjects[obj].model.animId;
			}, "ANIM");
			lua->CreateFunction([this](int obj) -> int {
				//TODO: realize
				return 0;
			}, "END_ANIM");
			lua->CreateFunction([this](int obj) -> int {
				//TODO: realize
				return 0;
			}, "POSREL");
			lua->CreateFunction([this](int obj) -> int {
				//TODO: realize
				return this->world->gobjects[obj].model.animId;
			}, "IS_FOUND");
		}

		void initInstructions() {
			lua->CreateFunction([this](const char* message) {
				cout << "LUA: " << message << endl;
			}, "LOG");
			lua->CreateFunction([this](int obj) {
				//
			}, "MESSAGE");
			lua->CreateFunction([this](int obj) {
				//
			}, "SET_MODEL");
			lua->CreateFunction([this](int obj) {
				//
			}, "SET_ANIM_ONCE");
			lua->CreateFunction([this](int obj) {
				//
			}, "SET_TRACKMODE");
			lua->CreateFunction([this](int obj) {
				//
			}, "SET_FLAGS");
			lua->CreateFunction([this](int obj) {
				//
			}, "SET_ANIM_SOUND");
			lua->CreateFunction([this](int obj) {
				//
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
				if (gobj.lifeIdx == -1) continue;
				auto& f = funcs.find(gobj.lifeIdx);
				if (f == funcs.end()) continue;

				string errstr;
				if (!f->second->Execute(execCb, &errstr, i)) {
					cout << "Execute life_" << i << " error: " << errstr;
				}
			}
		}

	};

}