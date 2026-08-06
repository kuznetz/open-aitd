#pragma once
#include <vector>
#include <string>
#include "../../common/raylib_cpp.hpp"
#include "../../common/metrics.hpp"
#include "../resources/resources.h"
#include "../../common/euler_angles.hpp"

using namespace std;
namespace openAITD {

	class GameObject;

	struct GOFlags
	{
		unsigned char animated : 1;
		unsigned char flag0x0002 : 1;
		unsigned char drawable : 1;
		unsigned char boxify : 1;
		unsigned char movable : 1;
		unsigned char special : 1; //collider?
		unsigned char trigger : 1;
		unsigned char foundable : 1;
		unsigned char fallable : 1;
		unsigned char flag0x0200 : 1;
		unsigned char flag0x0400 : 1;
		unsigned char flag0x0800 : 1;
		unsigned char flag0x1000 : 1;
		unsigned char flag0x2000 : 1;
		unsigned char flag0x4000 : 1;
		unsigned char flag0x8000 : 1;
	};

	struct InventoryFlags
	{
		unsigned char use : 1;
		unsigned char eat_drink : 1;
		unsigned char read : 1;
		unsigned char reload : 1;
		unsigned char fight : 1;
		unsigned char jump : 1;
		unsigned char open_search : 1;
		unsigned char close : 1;
		unsigned char push : 1;
		unsigned char throw_ : 1;
		unsigned char drop_put : 1;
		unsigned char flag0x0800 : 1;
		unsigned char throwing : 1;
		unsigned char flag0x2000 : 1;
		unsigned char dropped : 1;
		unsigned char in_inventory : 1;
	};

	struct AnimationFlags
	{
		unsigned char repeat : 1;
		unsigned char uninterruptable : 1;
		unsigned char reset : 1;
	};

    inline const BoundingBox defaultObjBounds = {
        { -0.1f, 0, -0.1f },
        { 0.1f, 2, 0.1f },
    };

	enum class BoundsType {
		simple = 1,
		cube = 2,
		rotated = 3
	};

	struct GOAnimation
	{
		int prevId = -1;
		int id = -1;
		int nextId = -1;
		Vector3 prevMoveRoot;
		Vector3 moveRoot;
		bool animChanged = false;
		int animEnd = 0; // 1 - last animation frame
		int scriptAnimId = -1; //scriptAnimId stay after change to next anim
		int animIdx = -1;
		int animFrame = -1;
		int keyFrameIdx;
		int keyFrameSoundIdx;
		float animTime;

		bool hasPose = false;
		vector<Transform> fromPose;
		vector<Transform> transitionPose;
		Transform* curPose;
		union {
			unsigned short int flags;
			AnimationFlags bitField;
		};
	};

	struct GORotateAnim {
		EulerAngles from;
		EulerAngles to;
		//Vector3i toOrig;
	  float curTime;
		float timeEnd = 0;
		// for life scripts
	};

	struct GOInvItem
	{
		int modelId = -1;
		int nameId = 0;
		int lifeId = 0;
		float foundTimeout = 0;
		union {
			unsigned short int flags;
			InventoryFlags bitField;
		};
	};

	struct GOPhysics
	{
		bool falling = 0;
		bool collidable = 1;
		bool moving;
		Vector3 moveVec;
		int collidedBy = -1;
		int staticColl = -1;
		int objectColl = -1;
		int zoneTriggered = -1;
		int hitObjectDamage = 0;
		bool boundsOverload = false;
		Bounds overloadBounds;
	};

	enum class GOLifeMode {
		none = -1,
		stage = 0,
		room = 1,
		roomInCamera = 2
	};

	enum class GOTrackMode {
		none = 0,
		manual = 1,
		follow = 2,
		track = 3
	};

  struct GOTrackDebug {
		Vector2 forward2D;
		Vector2 targetDir;
		float angle;
	};

	struct GOTrack {
		int mark = -1;
		int id = -1;
		int pos;
		GOTrackMode mode;
		bool posStarted = false;
		Vector3 startPos;
		Vector3 direction;
		Vector3 targetPos;
		GOTrackDebug debug;
	};

	struct GODamage
	{
		int damage;
		GameObject* hitBy = nullptr;
	};

	struct GOHit
	{
		bool active = false;
		float range;
		int boneIdx;
		int hitDamage;
		Bounds bounds;
		GameObject* hitTo = nullptr;
	};

	struct GOThrowing
	{
		bool active = false;
		Vector3 direction;
		GameObject* throwedBy = nullptr;
		int hitDamage;
	};

	class GameObject
	{
	public:
		int id = -1;
		string name;

		int prevModelId = -1;
		int modelId = -1;

		BoundsType boundsType = BoundsType::simple;

		GOAnimation animation;
		GORotateAnim rotateAnim;
		GOInvItem invItem;
		GOPhysics physics;
		GOTrack track;
		GODamage damage;
		GOHit hit;
		GOThrowing throwing;

		int staticColliderId = -1;
		RoomCollider* staticCollider = 0;

		union {
			short int flags;
			GOFlags bitField;
		};

		int stageLifeId;
		int lifeId;
		GOLifeMode lifeMode;	
		float chrono;

		GameObject(Resources& resources) :
		 resources(resources)
		 {}

    void setPosition(const Vector3& newPos) {
			position = newPos;
			boundsCached = false;
		}

		Vector3 getPosition() const {
			return position;
		}

		void setOrigRotation(const EulerAngles& newEuler){
			origRotation = newEuler.GetNormalized();
			rotMatrixCached = false;
			if (boundsType == BoundsType::rotated) {
				boundsCached = false;
			}			
		}

		Vector3i getIntRotation() const {
			//Reverse format for scripts			
			return Metrics::toRotate(origRotation);
		}		

		EulerAngles getOrigRotation() const {
			return origRotation;
		}

		Matrix getRotMatrix() {
			if (rotMatrixCached) {
				return rotMatrix;
			}
			EulerAngles rot = origRotation;
			rot = { -rot.x, -rot.y, -rot.z };
			rotMatrix = MatrixRotateYZX(rot);
			rotMatrixCached = true;
			return rotMatrix;
		}

		Bounds getBounds() {
			if (this->boundsCached) {
				return this->bounds;
			}
			RModel* m = resources.models.getModel(this->modelId);
			if (!m) return {{0,0,0},{0,0,0}};
			Bounds objB = (this->physics.boundsOverload) ? this->physics.overloadBounds : m->bounds;
			objB.correctBounds();

			if (this->boundsType == BoundsType::cube) {
				objB = objB.getCubeBounds();
			} else if (this->boundsType == BoundsType::rotated) {
				objB = objB.getRotatedBounds(this->getRotMatrix());
			}
			
			Vector3& p = this->position;
			objB.min = Vector3Add(objB.min, p);
			objB.max = Vector3Add(objB.max, p);
			objB.correctBounds();

			this->bounds = objB;
			this->boundsCached = true;
			return objB;
		}

		Bounds getRenderBounds() {
			RModel* m = resources.models.getModel(this->modelId);
			if (!m) return {{0,0,0},{0,0,0}};
		  m->model.CalcBounds();
			Bounds objB = m->model.bounds;

			Matrix matr = MatrixIdentity();
			matr = MatrixMultiply(MatrixRotateY(PI), matr);
			matr = MatrixMultiply(getRotMatrix(), matr);			
			objB = objB.getRotatedBounds(matr);

			Vector3 pos = this->position;
			const Vector3& roomPos = resources.stages[stageId].rooms[roomId].origPosition;
			pos = Vector3Add(pos, roomPos);
			objB.min = Vector3Add(objB.min, pos);
			objB.max = Vector3Add(objB.max, pos);
			objB.correctBounds();

			return objB;
		}		

		void setStage( const int stageId, const int roomId, const Vector3 position ) {
  		this->stageId = stageId;
			this->roomId = roomId;
			setPosition(position);
		}

		int getStageId() const {
			return stageId;
		}

  	void changeRoom( const int toRoomId ) {			
			auto& curStage = resources.stages[stageId];
			auto& roomFrom = curStage.rooms[roomId].origPosition;
			auto& roomTo = curStage.rooms[toRoomId].origPosition;
			position = Vector3Subtract(Vector3Add(position, roomFrom), roomTo);
			this->roomId = toRoomId;
		}

		int getRoomId() const {
			return roomId;
		}

		Bounds getBoundsInRoom(int toRoomId) {
			if (roomId == toRoomId) return getBounds();
			auto& curStage = resources.stages[stageId];
			auto& roomFrom = curStage.rooms[roomId].origPosition;
			auto& roomTo = curStage.rooms[toRoomId].origPosition;
			auto& b = getBounds();
			return {
				Vector3Subtract(Vector3Add(b.min, roomFrom), roomTo),
				Vector3Subtract(Vector3Add(b.max, roomFrom), roomTo)
			};
		}		

		bool changingStage = false;

	private:
	  Resources& resources;
	  //Position
	  Vector3 position;
		// Original rotation values YZX, need for scripts
		EulerAngles origRotation = { 0,0,0 };
		// Readonly - Current rotation matrix for physics and rendering
		raylib::Matrix rotMatrix;
		bool rotMatrixCached = false;
		Bounds bounds;
		bool boundsCached = false;
		//Stage
		int stageId = -1;
		int roomId = -1;
	};

}