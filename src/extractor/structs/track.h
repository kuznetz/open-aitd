#pragma once
#include "../structs/int_types.h"
#include <string>
#include <vector>

using namespace std;

namespace TrackEnum {
	enum TrackEnum
	{
		WARP,
		GOTO_POS,
		END,
		REWIND,
		MARK,
		SPEED_4,
		SPEED_5,
		SPEED_0,
		ROTATE_X,
		COLLISION_DISABLE,
		COLLISION_ENABLE,
		DUMMY,
		TRIGGERS_DISABLE,
		TRIGGERS_ENABLE,
		WARP_ROT,
		STORE_POS,
		STAIRS_X,
		STAIRS_Z,
		ROTATE_XYZ,
		UNUSED
	};
}

typedef struct {
	TrackEnum::TrackEnum type;
	string typeStr;
	int argumentCount;
} ParseTrackNode;

typedef struct {
	const ParseTrackNode* type = 0;
	vector<s16> arguments;
} TrackNode;

typedef vector<TrackNode> Track;
