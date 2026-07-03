#pragma once
#include "./life_core.hpp"
#include "../tracks_controller.h"
#include "../player_controller.h"
#include "../hit_controller.h"
#include "../throw_controller.h"
#include "../screens/found_screen.h"

#include <luacpp/luacpp.h>
#include <iostream>
#include <string>

using namespace std;
using namespace luacpp;

namespace openAITD {

	class LifeCommands {
	public:
	  LifeCore* lifeCore;
		World* world;
		Resources* resources;
		TracksController* trackContr;
		PlayerController* playerContr;
		HitController* hitContr;
		ThrowController* throwContr;
		FoundScreen* foundScreen;

		LuaState* lua = 0;

		LifeCommands(LifeCore* lifeCore, TracksController* trackContr, PlayerController* playerContr, HitController* hitContr, ThrowController* throwContr, FoundScreen* foundScreen) {
			this->lifeCore = lifeCore;
			this->world = lifeCore->world;
			this->resources = lifeCore->resources;
			this->trackContr = trackContr;
			this->playerContr = playerContr;
			this->hitContr = hitContr;
			this->throwContr = throwContr;
			this->foundScreen = foundScreen;
			
			lua = lifeCore->lua;
			initExpressions();
			initInstructions();
		}

		~LifeCommands() {
		}

		float getTimeDelta() {
			return lifeCore->curTimeDelta;
		}

		GameObject* getCurGObject() {
			return lifeCore->curGObj;
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
				//TODO: Camera index?
				return 0;
				}, "CAMERA");
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
			lua->CreateFunction([this](int obj, int animId, int offsetY) -> int {
				auto& gobj = this->world->gobjects[obj];
				bool result = canCompleteAnimation(gobj, animId, -(offsetY / 1000.));
				return result? 1: 0;
				}, "TEST_ZV_END_ANIM");

			lua->CreateFunction([this](int obj) -> int {
				return this->world->gobjects[obj].animation.keyFrameIdx;
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
					trackContr->processObjTrack(obj, this->getTimeDelta());
					break;
				case GOTrackMode::manual:
					playerContr->processObj(obj, this->getTimeDelta());
					break;
				case GOTrackMode::follow:
					trackContr->processObjFollow(obj, this->getTimeDelta());
					break;
				};
				}, "DO_MOVE");
			lua->CreateFunction([this](int objId) {
				auto& obj = this->world->gobjects[objId];
				playerContr->processRotate(obj, this->getTimeDelta());
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
			lua->CreateFunction([this]() {
				this->getCurGObject()->location.position.y += 2.001f;
				this->getCurGObject()->physics.boundsCached = false;
				//this->getCurGObject()->animation.moveRoot.y -= 3.0f;
				cout << "UP_COOR_Y" << endl;
				//TODO: UP_COOR_Y
				}, "UP_COOR_Y");

      lua->CreateFunction([this](int fromObjId) {
				auto& obj = this->world->gobjects[fromObjId];
				this->getCurGObject()->location.rotation = obj.location.rotation;
				}, "COPY_ANGLE");			

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
				resources->audio.PlayMusic(musicId);
				}, "SET_MUSIC");
			lua->CreateFunction([this](int musicId) {
				resources->audio.PlayMusic(musicId);
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

		bool canCompleteAnimation(GameObject& gobj, int animationId, float yOffset) {			
			if (gobj.modelId == -1) return false;
			RModel* m = resources->models.getModel(gobj.modelId);
			if (!m) return false;

			auto& animIter = m->animsIds.find(animationId);
			if (animIter == m->animsIds.end()) {
				return false;
			}
			
			auto& animIdx = animIter->second;
			raylib::Vector3 rootMotion = m->model.getLastFrameRootMotion(animIdx);
			raylib::Vector3 globalMotion = Vector3RotateByQuaternion(rootMotion, gobj.location.rotation);
			globalMotion.y += yOffset + 0.001f;

			Bounds newBounds = world->getObjectBounds(gobj);
			newBounds.min.x += globalMotion.x;
			newBounds.max.x += globalMotion.x;
			newBounds.min.y += globalMotion.y;
			newBounds.max.y += globalMotion.y;
			newBounds.min.z += globalMotion.z;
			newBounds.max.z += globalMotion.z;

			Room& room = resources->stages[world->curStageId].rooms[gobj.location.roomId];
			for (const auto& collider : room.colliders) {
					if (newBounds.CollToBox(collider.bounds)) {
							cout << "Collision!" << endl;
							return false;
					}
			}

			newBounds.min.y -= 0.1f;
			newBounds.max.y -= 0.1f;
			for (const auto& collider : room.colliders) {
					if (collider.type != 3) continue;
					if (newBounds.CollToBox(collider.bounds)) {
						  cout << "!!!collider.type " << collider.type << endl;
							return true;
					}
			}

			cout << "Collision, no floor!" << endl;
			return false;			
		}

	};

}
