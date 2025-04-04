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

			auto lfunc = lua->CreateFunction([](const char* msg) -> int {
				cout << "LUA: " << msg << endl;
				return 0;
			}, "LOG");
			auto lfunc2 = lua->CreateFunction([]() {
				cout << "TEST" << endl;
				return 5;
			}, "TEST");

			string errstr;
			if (!lua->DoFile("data/life/life_123.lua", &errstr)) {
				cout << "Load life_123 failed!";
			}

			testF = new LuaFunction(lua->GetFunction("life_123"));
			if (testF->GetType() != LUA_TFUNCTION) {
				cout << "life_123 is not function!";
			}
		}

		void process() {
			const std::function<bool(uint32_t, const LuaObject&)> callback([](uint32_t x, const LuaObject& lobj) -> bool {
				cout << "output from resiter1: " << lobj.ToString() << endl;
				return true;
			});
			string errstr;
			//lua->DoString("life_123(321)", &errstr);
			testF->Execute(callback, &errstr, 333);

			//auto& testF2 = lua->GetFunction("life_123");
			//testF2.Execute(callback, &errstr, 333);

		}

	};

}