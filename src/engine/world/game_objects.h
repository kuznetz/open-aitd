#pragma once
#include <vector>
#include <string>
#include "../raylib.h"

using namespace std;
namespace openAITD {

	struct GOLocation
	{
		int stageId = -1;
		int roomId = -1;
		Vector3 position;
		Vector4 rotation;
	};

	struct GOModel
	{
		int id = -1;
		int animId = -1;
		int animType = 0;
		int animInfo = 0;
		float animTime;
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