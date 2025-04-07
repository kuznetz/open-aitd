#pragma once
#include <vector>
#include <string>
#include "../raylib.h"

using namespace std;
namespace openAITD {

	struct GOFlags
	{
		char anim : 1;
		char flag0x0002 : 1;
		char flag0x0004 : 1;
		char back : 1;
		char push : 1;
		char coll : 1;
		char trig : 1;
		char pick : 1;
		char grav : 1;
		char flag0x0200 : 1;
		char flag0x0400 : 1;
		char flag0x0800 : 1;
		char flag0x1000 : 1;
		char flag0x2000 : 1;
		char flag0x4000 : 1;
		char flag0x8000 : 1;
	};

	struct InventoryFlags
	{
		char use : 1;
		char eat_drink : 1;
		char read : 1;
		char reload : 1;
		char fight : 1;
		char jump : 1;
		char open_search : 1;
		char close : 1;
		char push : 1;
		char throw_ : 1;
		char drop_put : 1;
		char flag0x0800 : 1;
		char throwing : 1;
		char flag0x2000 : 1;
		char dropped : 1;
		char in_inventory : 1;
	};

	struct AnimationFlags
	{
		char repeat : 1;
		char uninterruptable : 1;
		char reset : 1;
	};

    inline const BoundingBox defaultObjBounds = {
        { -0.1f, 0, -0.1f },
        { 0.1f, 2, 0.1f },
    };

	struct GOLocation
	{
		int stageId = -1;
		int roomId = -1;
		Vector3 position;
		Vector4 rotation;
	};

	struct GOModel
	{
		enum class BoundsType {
			simple = 1,
			cube = 2,
			rotated = 3
		};
		int id = -1;
		int animId = -1;
		int nextAnimId = -1;
		int animEnd = 1; //0 = First cycle after changing animaton from script
		int scriptAnimId = -1; //scriptAnimId stay after change to next anim
		int animFrame;
		float animTime;
		BoundsType boundsType = BoundsType::simple;
		union {
			unsigned short int flags;
			AnimationFlags bitField;
		};
	};

	struct GOInvItem
	{
		int model = -1;
		int name = 0;
		int life = 0;
		union {
			unsigned short int flags;
			InventoryFlags bitField;
		};
	};

	struct GOPhysics
	{
		bool boundsCached;
		BoundingBox bounds;
		Vector3 moveVec;
		int collidedBy = -1;
	};

	enum class GOLifeMode {
		none = -1,
		off = 0,
		stage = 1,
		room = 2,
		camera = 3
	};

	struct GOTrack {
		int mark = -1;
		int id = -1;
		int pos;
		int mode = -1;
	};

	class GameObject
	{
	public:
		int id = -1;

		GOLocation location;
		GOModel model;
		GOInvItem invItem;
		GOPhysics physics;
		GOTrack track;

		union {
			short int flags;
			GOFlags bitField;
		};

		int lifeId;
		GOLifeMode lifeMode;		

		bool moveFlag = false;
	};

}