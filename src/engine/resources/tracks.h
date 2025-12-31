#pragma once
#include <vector>
#include <string>
#include "../raylib-cpp.h"

using namespace std;
namespace openAITD {

	enum class TrackItemType {
		WARP,
		GOTO_POS,
		END,
		REWIND,
		MARK,
		SPEED_4,
		SPEED_5,
		SPEED_0,
		ROTATE_Y,
		COLLISION_DISABLE,
		COLLISION_ENABLE,
		DUMMY,
		TRIGGERS_DISABLE,
		TRIGGERS_ENABLE,
		GOTO_3D,
		STORE_POS,
		STAIRS_X,
		STAIRS_Z,
		ROTATE_XYZ,
		UNUSED
	};

	struct TrackItem
	{
		TrackItemType type;
		int room;
		int mark;
		float time;
		Vector3 pos;
		Vector3 rot;
	};

	typedef vector<TrackItem> Track;

}