#pragma once
struct objectFlags
{
	u8 anim : 1;
	u8 flag0x0002 : 1;
	u8 flag0x0004 : 1;
	u8 back : 1;
	u8 push : 1;
	u8 coll : 1;
	u8 trig : 1;
	u8 pick : 1;
	u8 grav : 1;
	u8 flag0x0200 : 1;
	u8 flag0x0400 : 1;
	u8 flag0x0800 : 1;
	u8 flag0x1000 : 1;
	u8 flag0x2000 : 1;
	u8 flag0x4000 : 1;
	u8 flag0x8000 : 1;
};

struct inventoryFlags
{
	u8 use         : 1;
	u8 eat_drink   : 1;
	u8 read        : 1; 
	u8 reload      : 1;
	u8 fight       : 1;
	u8 jump        : 1;
	u8 open_search : 1;
	u8 close       : 1;
	u8 push         : 1;
	u8 throw_       : 1;
	u8 drop_put     : 1;
	u8 flag0x0800   : 1;
	u8 throwing     : 1;
	u8 flag0x2000   : 1; 
	u8 dropped       : 1;
	u8 in_inventory : 1; 
};

//DELETE flags2 -in_inventory +placed
//FOUND_FLAG flags2 &= 2000|placed|in_inventory; flags2 |= argument
//THROW flags2 + throwing
//PUT flags2 + placed
//TAKE flags2 -placed +in_inventory
//IF_FOUND flags2 & in_inventory
//OBJECT?? flags2 & placed|in_inventory

struct gameObjectStruct
{
	short int ownerIdx;
	short int body;
	union {
		short int flags;
		objectFlags bitField;
	};
	short int boundsType;
	short int inventoryBody;
	short int inventoryName;
	union {
		short int invFlags;
		inventoryFlags invBitField;
	};
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