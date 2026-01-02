#pragma once
#include <vector>
#include <string>
#include "../raylib-cpp.h"
#include "../resources/resources.h"

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

	struct Vector3i {
		int x, y, z;
	};

	struct GOLocation
	{
		int stageId = -1;
		int roomId = -1;
		Vector3 position;
		Vector4 rotation;
		Vector3i rotOrig;
	};

	struct GOAnimation
	{
		int prevId = -1;
		int id = -1;
		int nextId = -1;
		Vector3 prevMoveRoot;
		Vector3 moveRoot;
		bool animChanged = false;
		int animEnd = 1; //0 = First cycle after changing animaton from script
		int scriptAnimId = -1; //scriptAnimId stay after change to next anim
		int animIdx;
		int animFrame;
		int keyFrameIdx;
		int keyFrameSoundIdx;
		float animTime;

		bool hasPose = false;
		vector<Transform> transitionPose;
		vector<Transform> curPose;
		union {
			unsigned short int flags;
			AnimationFlags bitField;
		};
	};

	struct GORotateAnim {
		Quaternion from;
		Quaternion to;
		Vector3i toOrig;
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
		bool boundsCached;
		bool falling = 0;
		bool collidable = 1;
		Bounds bounds;
		bool moving;
		Vector3 moveVec;
		int collidedBy = -1;
		int staticColl = -1;
		int objectColl = -1;
		int zoneTriggered = -1;
		int hitObjectDamage = 0;
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

	struct GOTrack {
		int mark = -1;
		int id = -1;
		int pos;
		GOTrackMode mode;
		bool posStarted = false;
		Vector3 startPos;
		Vector3 direction;
		Vector3 targetPos;
	};

	struct GODamage
	{
		int damage;
		GameObject* hitBy;
	};

	struct GOHit
	{
		bool active = false;
		float range;
		int boneIdx;
		int hitDamage;
		Bounds bounds;
		GameObject* hitTo;
	};

	struct GOThrowing
	{
		bool active = false;
		Vector3 direction;
		GameObject* throwedBy;
		int hitDamage;
	};

	class GameObject
	{
	public:
		int id = -1;

		//openAITD::Model* model;
		int prevModelId = -1;
		int modelId = -1;

		BoundsType boundsType = BoundsType::simple;

		GOLocation location;
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
	};

}