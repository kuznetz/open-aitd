#pragma once
//#include "../../resources/resources.h"
#include "../../world/world.h"

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

	class LifeCore {
	public:
		World* world;
		Resources* resources;

		LuaState* lua = 0;
		map <int, LifeFunc> funcs;
		std::function<bool(uint32_t, const LuaObject&)> execCb;
		GameObject* curGObj = 0;
		float curTimeDelta = 0;

		LifeCore(World* world) {
			this->world = world;
			this->resources = world->resources;
			initLua();
		}

		~LifeCore() {
			delete lua;
		}

		void initLua() {
			lua = new LuaState(luaL_newstate(), true);
			execCb = ([](uint32_t x, const LuaObject& lobj) -> bool {
				cout << "out: " << lobj.ToInteger() << endl;
				return true;
			});

			lua->CreateFunction([this](const char* message) {
				cout << "LUA: " << message << endl;
			  }, "LOG");


			string errstr;
			if (!lua->DoFile("data/constants.lua", &errstr)) {
				cout << "Load life failed: " << errstr;
			}
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

		void reloadVars() {
			for (int i = 0; i < world->vars.size(); i++) {
				string name = string("var_") + to_string(i);
				lua->CreateInteger(world->vars[i], name.c_str());
			}
		}

		vector<short> dumpVars() {
			vector<short> result;
			for (int i = 0; i < world->vars.size(); i++) {
				string name = string("var_") + to_string(i);
				result.push_back(lua->GetNumber(name.c_str()));
			}
			return result;
		}

		void executeLife(int lifeId, GameObject& gobj) {
			int objId = gobj.id;
			auto& f = funcs.find(lifeId);
			if (f == funcs.end()) {
				cout << "Life " << lifeId << " not found, (objId: " << objId << ")" << endl;
				return;
			}

  		curGObj = &gobj;
			string errstr;
			if (!f->second.func->Execute(execCb, &errstr, objId)) {
				cout << "Execute life_" << lifeId << " error: " << errstr << endl;
			}
			f->second.executed = true;
		}

		bool isObjectActive(GameObject& gobj) {
			if (gobj.lifeId == -1) return false;
			if (gobj.location.stageId != world->curStageId) return false;
			if (gobj.lifeMode == GOLifeMode::none) return false;
			if (gobj.lifeMode == GOLifeMode::room && gobj.location.roomId != world->curRoomId) return false;
			if (gobj.lifeMode == GOLifeMode::roomInCamera) {
				if (world->curCameraId == -1) return false;				
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

		void process(float timeDelta) {
			curTimeDelta = timeDelta;

			/*auto foll = world->followTarget;
			if (foll && (foll->location.stageId != -1) && (foll->location.stageId != world->curStageId || foll->location.roomId != world->curRoomId)) {
				world->setCurRoom(foll->location.stageId, foll->location.roomId);
			}*/

			for (auto it = funcs.begin(); it != funcs.end(); it++)
			{
				it->second.executed = false;
			}

			for (int i = 0; i < world->gobjects.size(); i++) {				
				auto& gobj = world->gobjects[i];
				if (!isObjectActive(gobj)) continue;
				executeLife(gobj.lifeId, gobj);
			}

			if (world->takedObj) {
				auto& gobj = world->takedObj;
				world->curInvAction = 2048;
				executeLife(gobj->invItem.lifeId, *gobj);
				world->curInvAction = 0;
				world->takedObj = 0;
			}

			if (world->curInvGObject) {
				executeLife(world->curInvGObject->invItem.lifeId, *world->curInvGObject);
				world->curInvGObject = 0;
				world->curInvAction = 0;
			}

			if (world->inHandObj) {
    		world->curInvAction = world->player.space ? 8192 : 0;
				auto& gobj = world->inHandObj;
				executeLife(gobj->invItem.lifeId, *gobj);
			}

			/*string s = "";
			for (auto it = funcs.begin(); it != funcs.end(); it++)
			{
				if (it->second.executed) s += " " + to_string(it->first);
			}*/

		}

	};

}
