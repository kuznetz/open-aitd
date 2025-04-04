#pragma once
#include <vector>
#include <string>
#include "../raylib.h"

using namespace std;
namespace openAITD {

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
		//don't save
		Vector3 moveVec;
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
		int animType = 0;
		int animInfo = 0;
		float animTime;
		BoundsType boundsType = BoundsType::simple;
		bool boundsCached;
		BoundingBox bounds;
	};

	struct GOInvItem
	{
		int ownerId = -1;
		int model = -1;
		int name = 0;
		int life = 0;
	};

	class GameObject
	{
	public:
		GOLocation location;
		GOModel model;
		GOInvItem invItem;

		int lifeIdx;
		int lifeMode;
		int traceIdx;
	};

}