#include "structs/int_types.h"
#include "pak/pak.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>

using namespace std;

typedef enum LifeOpArgType {
	eConst,
	eVar,
	eAddr,
	eActor,
} LifeOpArg;

typedef struct LifeOpCode {
	u8 code;
	string name;
	vector<LifeOpArgType> args;
} LifeOpCode;

typedef enum LifeEvalType
{
	NONE = -1,
	ACTOR_COLLIDER,
	TRIGGER_COLLIDER,
	HARD_COLLIDER,
	HIT,
	HIT_BY,
	ANIM,
	END_ANIM,
	FRAME,
	END_FRAME,
	BODY,
	MARK,
	NUM_TRACK,
	CHRONO,
	ROOM_CHRONO,
	DIST,
	COL_BY,
	ISFOUND,
	ACTION,
	POSREL,
	KEYBOARD_INPUT,
	SPACE,
	CONTACT,
	ALPHA,
	BETA,
	GAMMA,
	INHAND,
	HITFORCE,
	CAMERA,
	RAND,
	FALLING,
	ROOM,
	LIFE,
	OBJECT,
	ROOMY,
	TEST_ZV_END_ANIM,
	MUSIC,
	C_VAR,
	STAGE,
	THROW,
	MATRIX,
	HARD_MAT,
	PROTECT,
	END_WAY,
	DIV_BY_2,
	UNKNOWN
} LifeEvalType;

typedef struct LifeOperVar {
	LifeEvalType type;
	s16 value;
} LifeOp;

typedef struct LifeOper {
	const LifeOpCode* opCode = 0;
	vector<LifeOperVar> vars;
	vector<s16> consts;
	s16 egoto;
} LifeOp;


const vector<LifeOpCode> lifeOpCodes = {
	{ 0, "DO_MOVE", {} },
	{ 1, "ANIM_ONCE", { eVar, eVar } },
	{ 2, "ANIM_ALL_ONCE", { eVar, eVar } },
	{ 5, "IF_DIFFERENT", { eVar, eVar, eAddr } },
};

void evalVar(u8*& datap) {
	u16 evarTypeN = READ_LE_S16(datap);
	datap += 2;
	//const
	if (evarTypeN == -1) {
		u16 c = READ_LE_S16(datap);
		datap += 2;
		return;
	}
	if (evarTypeN == 0) {
		//variable
		u16 c = READ_LE_S16(datap);
		datap += 2;
		return;
	}
}

LifeOper readOperation(u8* &datap) {
	LifeOper result;
	u16 opCodeN = READ_LE_U16(datap);
	datap += 2;
	for (int i = 0; i < lifeOpCodes.size(); i++)
	{
		if (lifeOpCodes[i].code == opCodeN) {
			result.opCode = &lifeOpCodes[i];
			break;
		}
	}
	if (result.opCode == 0) throw new exception("Opcode not found");

	for (int i = 0; i < result.opCode->args.size(); i++)
	{
		switch (result.opCode->args[i]) {
		case eVar:
			evalVar(datap);
			break;

		case eConst:
		case eAddr:
		case eActor:
			auto a = result.opCode->args[i];
			s16 arg = READ_LE_S16(datap);
			datap += 2;
			break;
		}
	}

	return result;
}

void extractLife(string fname, string outDir)
{
	PakFile pak(fname);
	int i = 2;
	//for (int i = 0; i < pak.headers.size(); i++)
	{
		auto& data = pak.readBlock(i);
		auto datap = data.data();
		auto datap_end = datap + pak.headers[i].uncompressedSize;

		while (datap < datap_end) {
			auto& oper = readOperation(datap);
		}
		

/*
	while (pos < allBytes.Length)
	{
		int position = pos;

		int curr = allBytes.ReadShort(pos);
		int eActor = -1;
		if ((curr & 0x8000) == 0x8000)
		{
			curr &= 0x7FFF;
			pos += 2;
			eActor = allBytes.ReadShort(pos);
		}

		if (curr < 0 || curr >= config.LifeMacro.Length)
		{
			throw new IndexOutOfRangeException(curr.ToString());
		}
		LifeEnum life = config.LifeMacro[curr];

		Instruction ins = new Instruction
		{
			Type = life
		};

		if (eActor != -1) ins.Actor = GetObjectName(eActor);
		pos += 2;

		ParseArguments(life, ins);
		var node = nodes.AddLast(ins);
		nodesMap.Add(position, node);
		ins.Position = position;
		ins.Size = pos - position;
	}
*/

	}
}

/*{
	DO_MOVE,
	ANIM_ONCE,
	ANIM_ALL_ONCE,
	BODY,
	IF_EGAL,
	IF_DIFFERENT,
	IF_SUP_EGAL,
	IF_SUP,
	IF_INF_EGAL,
	IF_INF,
	GOTO,
	RETURN,
	ENDLIFE,
	ANIM_REPEAT,
	ANIM_MOVE,
	TRACKMODE,
	HIT,
	MESSAGE,
	MESSAGE_VALUE,
	VAR,
	INC,
	DEC,
	ADD,
	SUB,
	LIFE_MODE,
	SWITCH,
	CASE,
	CAMERA,
	START_CHRONO,
	MULTI_CASE,
	FOUND,
	LIFE,
	DELETE,
	TAKE,
	IN_HAND,
	READ,
	ANIM_SOUND,
	SPECIAL,
	DO_REAL_ZV,
	SOUND,
	FLAGS,
	GAME_OVER,
	MANUAL_ROT,
	RND_FREQ,
	MUSIC,
	SET_BETA,
	DO_ROT_ZV,
	CHANGEROOM,
	FOUND_NAME,
	FOUND_FLAG,
	FOUND_LIFE,
	CAMERA_TARGET,
	DROP,
	FIRE,
	TEST_COL,
	FOUND_BODY,
	SET_ALPHA,
	STOP_BETA,
	DO_MAX_ZV,
	PUT,
	C_VAR,
	DO_NORMAL_ZV,
	DO_CARRE_ZV,
	SOUND_THEN,
	LIGHT,
	SHAKING,
	ALLOW_INVENTORY,
	FOUND_WEIGHT,
	UP_COOR_Y,
	SPEED,
	PUT_AT,
	DEF_ZV,
	HIT_OBJECT,
	GET_HARD_CLIP,
	ANGLE,
	REP_SOUND,
	THROW,
	WATER,
	PICTURE,
	STOP_SOUND,
	NEXT_MUSIC,
	FADE_MUSIC,
	STOP_HIT_OBJECT,
	COPY_ANGLE,
	END_SEQUENCE,
	SOUND_THEN_REPEAT,
	WAIT_GAME_OVER,
	GET_MATRICE,
	STAGE_LIFE,
	CONTINUE_TRACK,
	ANIM_RESET,
	RESET_MOVE_MANUAL,
	PLUIE,
	ANIM_HYBRIDE_ONCE,
	ANIM_HYBRIDE_REPEAT,
	CALL_INVENTORY,
	BODY_RESET,
	DEL_INVENTORY,
	SET_INVENTORY,
	PLAY_SEQUENCE,
	_2D_ANIM_SOUND,
	SET_GROUND,
	PROTECT,
	DEF_ABS_ZV,
	DEF_SEQUENCE_SOUND,
	READ_ON_PICTURE,
	FIRE_UP_DOWN,
	DO_ROT_CLUT,
	STOP_CLUT
}*/