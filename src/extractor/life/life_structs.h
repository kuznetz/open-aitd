﻿#pragma once
#include "../structs/int_types.h"
#include <string>
#include <vector>

using namespace std;

namespace EvalEnum {
	enum EvalEnum
	{
		CONST,
		VAR,
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
	};
}

namespace LifeEnum {
	enum LifeEnum
	{
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
		STOP_CLUT,
		IF_IN,
		IF_OUT,
		UNKNOWN_1,
		SET_VOLUME_SOUND,
		UNKNOWN_2,
		UNKNOWN_3,
		FADE_IN_MUSIC,
		SET_MUSIC_VOLUME,
		MUSIC_AND_LOOP,
		MUSIC_A,
		MUSIC_B,
		START_FADE_IN_MUSIC,
		START_FADE_IN_MUSIC_THEN,
		START_FADE_IN_MUSIC_LOOP,
		FADE_OUT_MUSIC_STOP,
		MUSIC_ALTER_TEMPO,
		REP_SOUND_N_TIME,
		UNKNOWN_4,
		DEBUG,
		UNKNOWN_5,
		UNKNOWN_6,
		CASE_DEFAULT = 0x10000
	};
}

enum parseArgType {
	lifeConst,
	lifeExpr,
	lifeGoto
};

struct parseLifeExpr {
	EvalEnum::EvalEnum Type;
	string typeStr;
	vector<parseArgType> arguments;
};

struct parseLifeInstruction {
	LifeEnum::LifeEnum Type;
	string typeStr;
	vector<parseArgType> arguments;
};

typedef struct LifeExpr
{
	const parseLifeExpr* Type = 0;
	s16 Actor = -1;
	s16 constVal = 0;
	vector<LifeExpr> arguments;
} LifeExpr;

typedef struct LifeInstruction
{
	const parseLifeInstruction* Type = 0;
	vector<LifeExpr> arguments;
	s16 Actor = -1;
	int Goto = -1;
	int Position = -1; //used to map gotos to a given instruction
	int Size = -1;     //used to map gotos to a given instruction
} LifeInstruction;
