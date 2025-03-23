#include "../structs/track.h"

inline const vector<ParseTrackNode> TrackTable_v1 =
{
  {TrackEnum::WARP, "WARP", 4},
  {TrackEnum::GOTO_POS, "GOTO_POS", 3},
  {TrackEnum::END, "END", 0},
  {TrackEnum::REWIND, "REWIND", 0},
  {TrackEnum::MARK, "MARK", 1},
  {TrackEnum::SPEED_4, "SPEED_4", 0},
  {TrackEnum::SPEED_5, "SPEED_5", 0},
  {TrackEnum::SPEED_0, "SPEED_0", 0},
  {TrackEnum::UNUSED, "UNUSED", 0}, //set SPEED to -1
  {TrackEnum::ROTATE_X, "ROTATE_X", 1},
  {TrackEnum::COLLISION_DISABLE, "COLLISION_DISABLE", 0},
  {TrackEnum::COLLISION_ENABLE, "COLLISION_ENABLE", 0},
  {TrackEnum::UNUSED, "UNUSED", 0}, //do not do anything (increase offset by 2)
  {TrackEnum::TRIGGERS_DISABLE, "TRIGGERS_DISABLE", 0},
  {TrackEnum::TRIGGERS_ENABLE, "TRIGGERS_ENABLE", 0},
  {TrackEnum::WARP_ROT, "WARP_ROT", 5},
  {TrackEnum::STORE_POS, "STORE_POS", 0},
  {TrackEnum::STAIRS_X, "STAIRS_X", 3},
  {TrackEnum::STAIRS_Z, "STAIRS_Z", 3},
  {TrackEnum::ROTATE_XYZ, "ROTATE_XYZ",3 }
};

struct TrackBuffer {
	u8* data;
	u8* dataEnd;
};

inline s16 trackRead(TrackBuffer& buf) {
	if (buf.data >= buf.dataEnd) throw new exception("read16 over buffer size");
	s16 res = READ_LE_S16(buf.data);
	buf.data += 2;
	return res;
}

inline Track loadTrack(u8* data, int size)
{
	TrackBuffer buf = {
		data,
		data + size
	};
	Track result;
	while (buf.data < buf.dataEnd) {
		TrackNode tr;
		u16 opCodeN = trackRead(buf);
		auto& parse = TrackTable_v1[opCodeN];
		tr.type = &parse;
		tr.arguments.resize(parse.argumentCount);
		for (int i = 0; i < parse.argumentCount; i++) {
			tr.arguments[i] = trackRead(buf);
		}
		result.push_back(tr);
		if (tr.type->type == TrackEnum::END) break;
		if (tr.type->type == TrackEnum::REWIND) break;
	}
	return result;
}
