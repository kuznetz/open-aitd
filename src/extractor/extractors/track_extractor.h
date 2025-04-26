#include "../structs/track.h"
#include "../utils/room_matrix.h"

inline const vector<ParseTrackNode> TrackTable_v1 =
{
  {TrackEnum::WARP, "WARP", 4}, //0
  {TrackEnum::GOTO_POS, "GOTO_POS", 3},
  {TrackEnum::END, "END", 0},
  {TrackEnum::REWIND, "REWIND", 0},
  {TrackEnum::MARK, "MARK", 1},
  {TrackEnum::SPEED_4, "SPEED_4", 0},
  {TrackEnum::SPEED_5, "SPEED_5", 0},
  {TrackEnum::SPEED_0, "SPEED_0", 0},
  {TrackEnum::UNUSED, "UNUSED", 0}, //set SPEED to -1
  {TrackEnum::ROTATE_Y, "ROTATE_Y", 1},
  {TrackEnum::COLLISION_DISABLE, "COLLISION_DISABLE", 0}, //10
  {TrackEnum::COLLISION_ENABLE, "COLLISION_ENABLE", 0},
  {TrackEnum::UNUSED, "UNUSED", 0}, //do not do anything (increase offset by 2)
  {TrackEnum::TRIGGERS_DISABLE, "TRIGGERS_DISABLE", 0},
  {TrackEnum::TRIGGERS_ENABLE, "TRIGGERS_ENABLE", 0},
  {TrackEnum::GOTO_3D, "GOTO_3D", 5},
  {TrackEnum::STORE_POS, "STORE_POS", 0},
  {TrackEnum::STAIRS_X, "STAIRS_X", 3},
  {TrackEnum::STAIRS_Z, "STAIRS_Z", 3},
  {TrackEnum::ROTATE_XYZ, "ROTATE_XYZ",3 } //19
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

inline void extractTrack(Track track, string jsonTo) {
    json outJson = json::array();
    for (int i = 0; i < track.size(); i++)
    {
        auto& tr = track[i];
        json objJson = json::object();
        objJson["type"] = tr.type->type;

		Vector3 v;
		switch (tr.type->type)
		{
		case TrackEnum::WARP:
			objJson["room"] = tr.arguments[0];
			objJson["pos"] = json::array();
			v = Vector3Transform({ tr.arguments[1] / 1000.f , tr.arguments[2] / 1000.f, tr.arguments[3] / 1000.f }, roomMatrix);
			objJson["pos"][0] = v.x;
			objJson["pos"][1] = v.y;
			objJson["pos"][2] = v.z;
			break;

		case TrackEnum::GOTO_POS:
			objJson["room"] = tr.arguments[0];
			v = Vector3Transform({ tr.arguments[1] / 1000.f , 0, tr.arguments[2] / 1000.f }, roomMatrix);
			objJson["pos"] = json::array();
			objJson["pos"][0] = v.x;
			objJson["pos"][1] = 0;
			objJson["pos"][2] = v.z;
			break;

		case TrackEnum::MARK:
			objJson["mark"] = tr.arguments[0];
			break;

		case TrackEnum::ROTATE_Y:
			objJson["rot"] = json::array();
			objJson["rot"][0] = 0;
			objJson["rot"][1] = tr.arguments[0] * 2. * PI / 1024;
			objJson["rot"][2] = 0;
			break;

		case TrackEnum::GOTO_3D:
			objJson["room"] = tr.arguments[0];
			objJson["pos"] = json::array();
			objJson["pos"][0] = tr.arguments[1];
			objJson["pos"][1] = tr.arguments[2];
			objJson["pos"][2] = tr.arguments[3];
			objJson["time"] = tr.arguments[4] / 60.;
			break;

		case TrackEnum::STAIRS_X:
		case TrackEnum::STAIRS_Z:
			v = Vector3Transform({ tr.arguments[0] / 1000.f , tr.arguments[1] / 1000.f, tr.arguments[2] / 1000.f }, roomMatrix);
			objJson["pos"] = json::array();
			objJson["pos"][0] = v.x;
			objJson["pos"][1] = v.y;
			objJson["pos"][2] = v.z;
			break;

		case TrackEnum::ROTATE_XYZ:
			objJson["rot"] = json::array();
			objJson["rot"][0] = tr.arguments[0] * 2. * PI / 1024;
			objJson["rot"][1] = tr.arguments[1] * 2. * PI / 1024;
			objJson["rot"][2] = tr.arguments[2] * 2. * PI / 1024;
			break;
		}
        outJson.push_back(objJson);
    }

    std::ofstream o(jsonTo.c_str());
    o << std::setw(2) << outJson << std::endl;
}