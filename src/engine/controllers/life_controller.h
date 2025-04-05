#pragma once
#include "../resources/resources.h"
#include "../world/world.h"
#include <luacpp/luacpp.h>
#include <iostream>

using namespace std;
using namespace luacpp;

namespace openAITD {

	class LifeController {
	public:
		World* world;
		Resources* resources;
		LuaState* lua = 0;
		LuaFunction* testF;

		LifeController(Resources* res, World* world) {
			this->resources = res;
			this->world = world;
			initLua();
		}

		~LifeController() {
			delete lua;
		}

		void initExpressions() {
			auto lfunc = lua->CreateFunction([](const char* msg) -> int {
				cout << "LUA: " << msg << endl;
				return 0;
			}, "MODEL");
			auto lfunc = lua->CreateFunction([](const char* msg) -> int {
				cout << "LUA: " << msg << endl;
				return 0;
			}, "COL_BY");
			auto lfunc = lua->CreateFunction([](const char* msg) -> int {
				cout << "LUA: " << msg << endl;
				return 0;
			}, "ANIM");
			auto lfunc = lua->CreateFunction([](const char* msg) -> int {
				cout << "LUA: " << msg << endl;
				return 0;
			}, "END_ANIM");
			auto lfunc = lua->CreateFunction([](const char* msg) -> int {
				cout << "LUA: " << msg << endl;
				return 0;
			}, "POSREL");
			auto lfunc = lua->CreateFunction([](const char* msg) -> int {
				cout << "LUA: " << msg << endl;
				return 0;
			}, "POSREL");
			/*
			GET_MODEL
			COL_BY
			GET_ANIM
			END_ANIM
			POSREL
			OBJECT
			*/
		}

		void initInstructions() {
			auto lfunc = lua->CreateFunction([](const char* msg) -> int {
				cout << "LUA: " << msg << endl;
				return 0;
			}, "LOG");
			/*
			MESSAGE
			SET_MODEL
			ANIM_ONCE
			ANIM_SOUND
			TRACKMODE
			SET
			FLAGS
			FOUND
			*/
		}

		void initLua() {
			lua = new LuaState(luaL_newstate(), true);

			initExpressions();
			initInstructions();

			//"data/life/life_123.lua"
			string errstr;
			if (!lua->DoFile("data/scripts.lua", &errstr)) {
				cout << "Load life failed: " << errstr;
			}
			testF = new LuaFunction(lua->GetFunction("life_0"));
			if (testF->GetType() != LUA_TFUNCTION) {
				cout << "life_0 is not function!";
			}
		}

		void process() {
			const std::function<bool(uint32_t, const LuaObject&)> callback([](uint32_t x, const LuaObject& lobj) -> bool {
				cout << "output from resiter1: " << lobj.ToString() << endl;
				return true;
			});
			string errstr;
			if (!testF->Execute(callback, &errstr, 333)) {
				cout << "Execute life_0 error: " << errstr;				
			}

			//auto& testF2 = lua->GetFunction("life_123");
			//testF2.Execute(callback, &errstr, 333);

		}

	};

}