#pragma once
#include "../resources/resources.h"
#include "../world/world.h"
#include "./tracks_controller.h"
#include "./player_controller.h"
#include "./hit_controller.h"
#include "./throw_controller.h"
#include "../screens/found_screen.h"

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
		TracksController* trackContr;
		PlayerController* playerContr;
		HitController* hitContr;
		ThrowController* throwContr;
		FoundScreen* foundScreen;
		LuaState* lua = 0;
		map <int, LifeFunc> funcs;
		std::function<bool(uint32_t, const LuaObject&)> execCb;
		Matrix roomMatrix;
		float curTimeDelta = 0;

		LifeController(World* world, TracksController* trackContr, PlayerController* playerContr, HitController* hitContr, ThrowController* throwContr, FoundScreen* foundScreen) {
			this->world = world;
			this->trackContr = trackContr;
			this->playerContr = playerContr;
			this->hitContr = hitContr;
			this->throwContr = throwContr;
			this->foundScreen = foundScreen;
			this->resources = world->resources;
			initLua();
		}

		~LifeController() {
			delete lua;
		}

		int getPosRel(GameObject* actor1, GameObject* actor2)
		{
			if (actor1->staticCollider != 0) {
				return getPosRelStatic(actor1, actor2);
			}

			Vector3 p2rot;
			Bounds b;
			Vector3& p1 = actor1->location.position;
			Vector3& p2 = world->VectorChangeRoom(actor2->location.position, actor2->location.roomId, actor1->location.roomId);

			p2rot = { p2.x - p1.x, 0, p2.z - p1.z };
			p2rot = Vector3RotateByQuaternion(p2rot, QuaternionInvert(actor1->location.rotation)); //QuaternionInvert(actor1->location.rotation)
			p2rot = Vector3Add(p2rot, p1);
			RModel* m = resources->models.getModel(actor1->modelId);
			b = m->bounds;
			b.min = Vector3Add(b.min, p1);
			b.max = Vector3Add(b.max, p1);
			
			int res = 0;
			if (p2rot.z < b.min.z) {
				res = 2;
			}
			else if (p2rot.z > b.max.z) {
				res = 1;
			}
			else if (p2rot.x < b.min.x) {
				res = 8;
			}
			else if (p2rot.x > b.max.x) {
				res = 4;
			}
			//cout << "POSREL=" << to_string(res) << endl;
			return res;
		}

		int getPosRelStatic(GameObject* actor1, GameObject* actor2)
		{
			auto& b = actor1->staticCollider->bounds;
			Vector3& p2 = world->VectorChangeRoom(actor2->location.position, actor2->location.roomId, actor1->location.roomId);

			int res = 0;
			if (p2.z < b.min.z) {
				res = 1;
			}
			else if (p2.z > b.max.z) {
				res = 2;
			}
			else if (p2.x < b.min.x) {
				res = 8;
			}
			else if (p2.x > b.max.x) {
				res = 4;
			}
			//cout << "POSREL_S=" << to_string(res);
			return res;
		}

		Vector4 convertAngle(int alpha, int beta, int gamma) {
			auto roomMatObj = MatrixRotateX(PI);
			Matrix mx = MatrixRotateX(alpha * 2.f * PI / 1024);
			Matrix my = MatrixRotateY((beta + 512) * 2.f * PI / 1024);
			Matrix mz = MatrixRotateZ(gamma * 2.f * PI / 1024);
			Matrix matRotation = MatrixMultiply(MatrixMultiply(my, mx), mz);
			matRotation = MatrixTranspose(matRotation);
			auto q = QuaternionFromMatrix(matRotation);
			return QuaternionTransform(q, roomMatObj);
			//auto rotTo = QuaternionFromAxisAngle({ 0,1,0 }, (toAngle + 512) * 2. * PI / 1024.);
		}

		int convertAngle2(int oldAngle, int newAngle) {
			auto angleDif = (newAngle - oldAngle);
			//printf("SET_BETA DIFF %d\n", angleDif);			
			if (angleDif == 512)
			{
				angleDif -= 1;
			}
			else if (angleDif == -512)
			{
				angleDif += 1;
			}
			return oldAngle + angleDif;
		}

		void initExpressions() {
			lua->CreateFunction([this](int rmax) -> int {
				return rand() % rmax;
				}, "RAND");
			lua->CreateFunction([this](int i) -> int {
				return this->world->cVars[i];
				}, "GET_C");

			//player
			lua->CreateFunction([this](int obj) -> int {
				return this->world->player.space? 1: 0;
				}, "SPACE");
			lua->CreateFunction([this](int obj) -> int {
				return this->world->player.keyboard;
				}, "KEYBOARD_INPUT");
			lua->CreateFunction([this](int obj) -> int {
				auto& gobj = this->world->gobjects[obj];
				if (!gobj.bitField.fallable) return 0;
				return (int)this->world->gobjects[obj].physics.falling;
				}, "FALLING");
			lua->CreateFunction([this](int obj) -> int {
				return 1; //TODO: MUSIC
				}, "MUSIC");

			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].location.stageId;
				}, "STAGE");
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
				auto& gobj = this->world->gobjects[obj];
				int temp1 = gobj.physics.objectColl;
				if (temp1 == -1) {
					temp1 = gobj.physics.collidedBy;
				}
				return temp1;
				}, "CONTACT");
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
				auto hb = this->world->gobjects[obj].damage.hitBy;
				return hb ? hb->id : -1;
				}, "HIT_BY");
			lua->CreateFunction([this](int obj) -> int {
				auto ht = this->world->gobjects[obj].hit.hitTo;
				return ht ? ht->id : -1;
				}, "HIT_TO");
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].damage.damage;
				}, "HIT_DAMAGE");

			lua->CreateFunction([this](int obj) -> int {
				//return this->world->gobjects[obj].animation.scriptAnimId;
				return this->world->gobjects[obj].animation.id;
				}, "ANIM");
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].animation.animEnd;
				}, "END_ANIM");
			lua->CreateFunction([this](int obj, int animId, int param) -> int {
				//TODO: For climbing
				return 0;
				}, "TEST_ZV_END_ANIM");

			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].animation.animFrame;
				}, "FRAME");
			lua->CreateFunction([this](int obj, int obj2) -> int {
				return getPosRel(&this->world->gobjects[obj], &this->world->gobjects[obj2]);
				}, "POSREL");
			lua->CreateFunction([this](int obj1, int obj2) -> int {
				auto& gobj1 = this->world->gobjects[obj1].location.position;
				auto& gobj2 = this->world->gobjects[obj2].location.position;
				int x = abs(gobj1.x - gobj2.x) * 1000.;
				int y = abs(gobj1.y - gobj2.y) * 1000.;
				int z = abs(gobj1.z - gobj2.z) * 1000.;
				return x + y + z;
				}, "DIST");
			lua->CreateFunction([this](int obj) -> int {
				//int r = (this->world->gobjects[obj].invItem.flags & 0xC000) ? 1 : 0;
				int r = (this->world->gobjects[obj].bitField.foundable) ? 1 : 0;
				return r;
				}, "IS_FOUND");
			lua->CreateFunction([this](int obj) {
				return this->world->gobjects[obj].invItem.bitField.in_inventory;
				}, "IN_INVENTORY");
			lua->CreateFunction([this]() -> int {
				return this->world->curInvAction;
				}, "ACTION");
			lua->CreateFunction([this]() {
				return this->world->inHandObj->id;
				}, "IN_HAND");
			
			lua->CreateFunction([this](int obj) -> int {
				return (this->world->gobjects[obj].track.mark);
				}, "MARK");
			lua->CreateFunction([this](int obj) -> int {
				return (this->world->gobjects[obj].track.id);
				}, "NUM_TRACK");

			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].location.rotOrig.x;
				}, "ALPHA");
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].location.rotOrig.y;
				}, "BETA");
			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].location.rotOrig.z;
				}, "GAMMA");
			lua->CreateFunction([this](int obj) -> int {
				return floor(this->world->gobjects[obj].location.position.y * 1000);
				}, "ROOMY");			

			lua->CreateFunction([this](int obj) -> int {
				return (int)((this->world->chrono - this->world->gobjects[obj].chrono));
				}, "CHRONO");
			lua->CreateFunction([this](int obj) -> int {
				return (int)((this->world->chrono - this->world->roomChrono));
				}, "ROOM_CHRONO");
		}

		void initInstructions() {
			lua->CreateFunction([this](const char* message) {
				cout << "LUA: " << message << endl;
				}, "LOG");
			
			lua->CreateFunction([this](int i, int val) {
				world->cVars[i] = val;
				}, "SET_C");
			lua->CreateFunction([this](int messId) {
				this->world->messageText = resources->texts[messId];
				this->world->messageTime = 5;
				}, "MESSAGE");
			lua->CreateFunction([this](int obj, int a1, int a2, int a3, int a4, int a5, int a6, int a7) {
				auto& a = this->world->player.animations;
				a.idle = a1;
				a.walkForw = a2;
				a.runForw = a3;
				a.idle2 = a4;
				a.walkBackw = a5;
				a.turnCW = a6;
				a.turnCCW = a7;
				}, "SET_ANIM_MOVE");

			//Basic
			lua->CreateFunction([this](int obj, int modelId) {
				this->world->setModel(this->world->gobjects[obj], modelId);
			}, "SET_MODEL");
			lua->CreateFunction([this](int obj, int lifeId) {
				this->world->gobjects[obj].lifeId = lifeId;
			}, "SET_LIFE");
			lua->CreateFunction([this](int obj, int lifeMode) {
				this->world->gobjects[obj].lifeMode = GOLifeMode(lifeMode);
			}, "SET_LIFE_MODE");
			lua->CreateFunction([this](int obj) {
				this->world->gobjects[obj].chrono = this->world->chrono;
			}, "START_CHRONO");
			lua->CreateFunction([this](int obj, int coll) {
				this->world->gobjects[obj].physics.collidable = coll;
			}, "TEST_COL");
			//Set object rotation(angle)
			lua->CreateFunction([this](int obj, int x, int y, int z) {
				this->world->gobjects[obj].location.rotation = convertAngle(x, y, z);
				this->world->gobjects[obj].physics.boundsCached = false;
			}, "SET_ANGLE");
			lua->CreateFunction([this](int obj, int flags) {
				auto& gobj = this->world->gobjects[obj];
				gobj.flags = flags;
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
				gobj.location.position.x = x / 1000.;
				gobj.location.position.y = (-y / 1000.) + 0.001;
				gobj.location.position.z = -z / 1000.;
			}, "CHANGE_ROOM");

			//INVENTORY
			lua->CreateFunction([this](int allow) {
				this->world->player.allowInventory = !!allow;
				}, "ALLOW_INVENTORY");
			lua->CreateFunction([this](int obj) {
				auto& gobj = this->world->gobjects[obj];
				if (gobj.invItem.foundTimeout > this->world->chrono) return;
				if (gobj.invItem.bitField.in_inventory) return;
				foundScreen->main(obj);
				}, "FOUND");
			lua->CreateFunction([this](int obj, int nameId) {
				auto& gobj = this->world->gobjects[obj];
				gobj.invItem.nameId = nameId;
				}, "SET_INVENTORY_NAME");
			lua->CreateFunction([this](int obj, int newModelId) {
				auto& gobj = this->world->gobjects[obj];
				gobj.invItem.modelId = newModelId;
				}, "SET_INVENTORY_MODEL");			
			lua->CreateFunction([this](int obj, int newFlag) {
				auto& gobj = this->world->gobjects[obj];
				gobj.invItem.flags = newFlag;
				}, "SET_INVENTORY_FLAG");
			lua->CreateFunction([this](int obj) {
				return this->world->inHandObj = &this->world->gobjects[obj];
				}, "SET_IN_HAND");
			lua->CreateFunction([this](int obj) {
				world->take(obj);
				}, "TAKE");
			lua->CreateFunction([this](int itemObjId, int x, int y, int z, int room, int stage, int alpha, int beta, int gamma) {
				this->world->put(itemObjId, stage, room, { x / 1000.f, -y / 1000.f, -z / 1000.f }, convertAngle(alpha, beta, gamma));
				}, "PUT");
			lua->CreateFunction([this](int itemObjId, int targetObjId) {
				auto& gobj = this->world->gobjects[itemObjId];
				auto& tobj = this->world->gobjects[targetObjId];
				this->world->put(itemObjId, tobj.location.stageId, tobj.location.roomId, tobj.location.position, tobj.location.rotation);
				gobj.bitField.foundable = 1;
				}, "PUT_AT");
			lua->CreateFunction([this](int itemObjId, int actorObjId) {
				this->world->drop(itemObjId, actorObjId);
				}, "DROP");
			lua->CreateFunction([this](int obj, int animThrow, int frameThrow, int activeBone, int itemObjId, int throwRotated, int hitDamage, int animNext) {
				auto gobj = &this->world->gobjects[obj];
				auto gobj2 = &this->world->gobjects[itemObjId];
				this->world->setOnceAnimation(*gobj, animThrow, animNext, true);
				this->throwContr->throw_(gobj, gobj2, animThrow, frameThrow, frameThrow, activeBone, hitDamage);
				}, "THROW");
			lua->CreateFunction([this](int obj) {
				auto& gobj = this->world->gobjects[obj];				
				return gobj.throwing.active;
				}, "THROWING");
			lua->CreateFunction([this](int objId, int weight) {
				//Weight not realising
				}, "SET_OBJ_WEIGHT");
			lua->CreateFunction([this]() {
				//TODO: Read
				}, "READ");			

			//Animations, tracks, rotations
			lua->CreateFunction([this](int obj, int animId, int nextAnimId) {
				this->world->setOnceAnimation(this->world->gobjects[obj], animId, nextAnimId);
				}, "SET_ANIM_ONCE");
			lua->CreateFunction([this](int obj, int animId) {
				this->world->setRepeatAnimation(this->world->gobjects[obj], animId);
				}, "SET_ANIM_REPEAT");
			lua->CreateFunction([this](int obj, int animId, int nextAnimId) {
				this->world->setOnceAnimation(this->world->gobjects[obj], animId, nextAnimId, true);
				//ANIM_ONCE | ANIM_UNINTERRUPTABLE;
				}, "SET_ANIM_ALL_ONCE");
			
			//Actions
			lua->CreateFunction([this](int obj, int anim1, int keyFrameIdx, int activeBone, int range, int damage, int anim2) {
				auto gobj = &this->world->gobjects[obj];
				this->world->setOnceAnimation(this->world->gobjects[obj], anim1, anim2);
				this->hitContr->addAction(gobj, anim1, keyFrameIdx);
				gobj->hit.boneIdx = activeBone;
				gobj->hit.range = range / 1000.f;
				gobj->hit.hitDamage = damage;
				}, "HIT");
			lua->CreateFunction([this](int obj, int fireAnim, int shootFrame, int emitPoint, int zvSize, int damage, int nextAnim) {
				this->world->setOnceAnimation(this->world->gobjects[obj], fireAnim, nextAnim);
				}, "FIRE");
			lua->CreateFunction([this](int obj, int flags, int damage) {
				auto gobj = &this->world->gobjects[obj];
				gobj->physics.hitObjectDamage = damage;
				}, "HIT_OBJECT");
			lua->CreateFunction([this](int obj) {
				auto gobj = &this->world->gobjects[obj];
				gobj->physics.hitObjectDamage = 0;
				}, "STOP_HIT_OBJECT");
			

			lua->CreateFunction([this](int obj, int trackMode, int trackId, int positionInTrack) {
				auto& gobj = this->world->gobjects[obj];
				gobj.track.mode = GOTrackMode(trackMode);
				gobj.track.id = trackId;
				gobj.track.pos = positionInTrack;
				gobj.track.posStarted = false;
				if (gobj.track.mode == GOTrackMode::manual) {
					world->setRepeatAnimation(gobj, world->player.animations.idle);
				}
				}, "SET_TRACKMODE");

			lua->CreateFunction([this](int obj, int toAngle, int time) {
				auto& gobj = this->world->gobjects[obj];
				if (gobj.rotateAnim.timeEnd > 0 && gobj.rotateAnim.toOrig.x == toAngle) return;
				gobj.rotateAnim.curTime = 0;
				gobj.rotateAnim.timeEnd = time / 60.;
				gobj.rotateAnim.from = gobj.location.rotation;

				auto& ro = gobj.location.rotOrig;
				auto& ro2 = gobj.rotateAnim.toOrig;
				ro2 = ro;
				ro2.x = toAngle;

				gobj.rotateAnim.to = convertAngle(
					convertAngle2(ro.x, -toAngle),
					ro2.y,
					ro2.z
				);

				}, "SET_ALPHA");

			lua->CreateFunction([this](int obj, int toAngle, int time) {
				auto& gobj = this->world->gobjects[obj];
				if (gobj.rotateAnim.timeEnd > 0 && gobj.rotateAnim.toOrig.y == toAngle) return;
				gobj.rotateAnim.curTime = 0;
				gobj.rotateAnim.timeEnd = time / 60.;
				gobj.rotateAnim.from = gobj.location.rotation;

				auto& ro = gobj.location.rotOrig;
				auto& ro2 = gobj.rotateAnim.toOrig;
				ro2 = ro;
				ro2.y = toAngle;

				gobj.rotateAnim.to = convertAngle(
					ro2.x, 
					convertAngle2(ro.y, toAngle),
					ro2.z
				);

				}, "SET_BETA");

			//Process track
			lua->CreateFunction([this](int objId) {
				auto& obj = this->world->gobjects[objId];
				switch (obj.track.mode) {
				case GOTrackMode::track:
					trackContr->processObjTrack(obj, curTimeDelta);
					break;
				case GOTrackMode::manual:
					playerContr->processObj(obj, curTimeDelta);
					break;
				case GOTrackMode::follow:
					trackContr->processObjFollow(obj, curTimeDelta);
					break;
				};
				}, "DO_MOVE");
			lua->CreateFunction([this](int objId) {
				auto& obj = this->world->gobjects[objId];
				playerContr->processRotate(obj, curTimeDelta);
				}, "MANUAL_ROT");			
			lua->CreateFunction([this](int obj) {
				//Not needed
				}, "DO_ROT_ZV");
			//recalc bounds?
			lua->CreateFunction([this]() {
				//TODO: DEF_ZV
				}, "DEF_ZV");
			lua->CreateFunction([this]() {
				//TODO: DO_MAX_ZV
				}, "DO_MAX_ZV");
			lua->CreateFunction([this]() {
				//TODO: DO_CARRE_ZV
				}, "DO_CARRE_ZV");
			lua->CreateFunction([this]() {
				//TODO: DO_REAL_ZV
				}, "DO_REAL_ZV");
			lua->CreateFunction([this]() {
				//Do nothing?
				}, "GET_HARD_CLIP");

			//Sound & music
			lua->CreateFunction([this](int obj, int sampleId, int animId, int animFrame) {
				//TODO: remember frame, not play in life
				GOAnimation& curAnim = this->world->gobjects[obj].animation;
				if (curAnim.id != animId) return;
				if (curAnim.keyFrameSoundIdx == animFrame) return;
				if (curAnim.keyFrameIdx != animFrame) return;
				curAnim.keyFrameSoundIdx = animFrame;
				resources->audio.PlaySound(sampleId);
				}, "SET_ANIM_SOUND");
			lua->CreateFunction([this](int sampleId) {
				//cout << "SOUND " << sampleId << endl;
				resources->audio.PlaySound(sampleId);
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
				//TODO: SET_MUSIC
				}, "SET_MUSIC");
			lua->CreateFunction([this](int musicId) {
				//TODO: NEXT_MUSIC
				}, "NEXT_MUSIC");			
			lua->CreateFunction([this](int musicId) {
				//TODO: SOUND_THEN
				}, "FADE_MUSIC");

			//SFX
			lua->CreateFunction([this](int picId, int delay, int sampleId) {
				this->world->picture.id = picId;
				this->world->picture.curTime = 0;
				this->world->picture.delay = delay / 30.;
				resources->audio.PlaySound(sampleId);
				}, "PICTURE");			
			lua->CreateFunction([this](int light) {
				this->world->inDark = !light;
				}, "SET_LIGHT");
			lua->CreateFunction([this](int water) {
				//TODO: SOUND_THEN
				}, "WATER");
			lua->CreateFunction([this](int shaking) {
				//TODO: SET_SHAKING
				}, "SET_SHAKING");
			lua->CreateFunction([this](int obj, int spType) {
				//TODO: SPECIAL
				}, "SPECIAL");

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

		void executeLife(int lifeId, int objId) {
			auto& f = funcs.find(lifeId);
			if (f == funcs.end()) {
				cout << "Life " << lifeId << " not found, (objId: " << objId << ")" << endl;
				return;
			}

			string errstr;
			if (!f->second.func->Execute(execCb, &errstr, objId)) {
				cout << "Execute life_" << lifeId << " error: " << errstr << endl;
			}
			f->second.executed = true;
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
				executeLife(gobj.lifeId, i);
			}

			if (world->takedObj) {
				auto gobj = world->takedObj;
				world->curInvAction = 2048;
				executeLife(gobj->invItem.lifeId, gobj->id);
				world->curInvAction = 0;
				world->takedObj = 0;
			}

			if (world->curInvGObject) {
				executeLife(world->curInvGObject->invItem.lifeId, world->curInvGObject->id);
				world->curInvGObject = 0;
				world->curInvAction = 0;
			}

			if (world->inHandObj) {
    			world->curInvAction = world->player.space ? 8192 : 0;
				auto gobj = world->inHandObj;
				executeLife(gobj->invItem.lifeId, gobj->id);
			}

			/*string s = "";
			for (auto it = funcs.begin(); it != funcs.end(); it++)
			{
				if (it->second.executed) s += " " + to_string(it->first);
			}*/

		}

	};

}

//LifeEnum::DO_MOVE, +
//LifeEnum::ANIM_ONCE, +
//LifeEnum::ANIM_ALL_ONCE, +
//LifeEnum::SET_MODEL, +
//LifeEnum::SET_ANIM_REPEAT,
//LifeEnum::SET_ANIM_MOVE,
//LifeEnum::SET_TRACKMODE,
//LifeEnum::HIT,
//LifeEnum::MESSAGE,
//LifeEnum::SET_LIFE_MODE,
//LifeEnum::START_CHRONO,
//LifeEnum::FOUND,
//LifeEnum::SET_LIFE,
//LifeEnum::DELETE_OBJ,
//LifeEnum::TAKE,
//LifeEnum::IN_HAND,
//LifeEnum::READ,
//LifeEnum::SET_ANIM_SOUND,
//LifeEnum::SPECIAL,
//LifeEnum::DO_REAL_ZV,
//LifeEnum::SOUND,
//LifeEnum::SET_FLAGS,
//LifeEnum::GAME_OVER,
//LifeEnum::MANUAL_ROT,
//LifeEnum::RND_FREQ,
//LifeEnum::SET_MUSIC,
//LifeEnum::SET_BETA,
//LifeEnum::DO_ROT_ZV,
//LifeEnum::CHANGE_ROOM,
//LifeEnum::SET_INVENTORY_NAME,
//LifeEnum::SET_INVENTORY_FLAG,
//LifeEnum::SET_INVENTORY_LIFE,
//LifeEnum::SET_CAMERA_TARGET,
//LifeEnum::DROP,
//LifeEnum::FIRE,
//LifeEnum::TEST_COL,
//LifeEnum::SET_INVENTORY_MODEL,
//LifeEnum::SET_ALPHA,
//LifeEnum::DO_MAX_ZV,
//LifeEnum::PUT,
//LifeEnum::SET_C,
//LifeEnum::DO_NORMAL_ZV,
//LifeEnum::DO_CARRE_ZV,
//LifeEnum::SOUND_THEN,
//LifeEnum::SET_LIGHT,
//LifeEnum::SET_SHAKING,
//LifeEnum::ALLOW_INVENTORY,
//LifeEnum::SET_OBJ_WEIGHT,
//LifeEnum::UP_COOR_Y,
//LifeEnum::PUT_AT,
//LifeEnum::DEF_ZV,
//LifeEnum::HIT_OBJECT,
//LifeEnum::GET_HARD_CLIP,
//LifeEnum::SET_ANGLE,
//LifeEnum::REP_SOUND,
//LifeEnum::THROW,
//LifeEnum::WATER,
//LifeEnum::PICTURE,
//LifeEnum::STOP_SOUND,
//LifeEnum::NEXT_MUSIC,
//LifeEnum::FADE_MUSIC,
//LifeEnum::STOP_HIT_OBJECT,
//LifeEnum::COPY_ANGLE,
//LifeEnum::END_SEQUENCE,
//LifeEnum::SOUND_THEN_REPEAT,
//LifeEnum::WAIT_GAME_OVER