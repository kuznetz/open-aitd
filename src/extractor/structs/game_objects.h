#pragma once
struct actorFlags
{
	u8 flag0x0001 : 1;
	u8 flag0x0002 : 1;
	u8 flag0x0004 : 1;
	u8 flag0x0008 : 1;
	u8 flag0x0010 : 1;
	u8 flag0x0020 : 1;
	u8 flag0x0040 : 1;
	u8 tackable : 1;
	u8 flag0x0100 : 1;
	u8 flag0x0200 : 1;
	u8 flag0x0400 : 1;
	u8 flag0x0800 : 1;
	u8 flag0x1000 : 1;
	u8 flag0x2000 : 1;
	u8 flag0x4000 : 1;
	u8 flag0x8000 : 1;
};

struct gameObjectStruct
{
	short int ownerIdx;
	short int body;
	union
	{
		short int flags;
		actorFlags bitField;
	};
	short int boundsType;
	short int inventoryBody;
	short int inventoryName;
	short int flags2;
	short int inventoryLife;
	short int x;
	short int y;
	short int z;
	short int alpha;
	short int beta;
	short int gamma;
	short int stageId;
	short int roomId;
	short int lifeMode;
	short int life;
	short int stageLife;
	short int anim;
	short int frame;
	short int animType;
	short int animInfo;
	short int trackMode;
	short int trackNumber;
	short int positionInTrack;

	// AITD2
	short int mark;
};