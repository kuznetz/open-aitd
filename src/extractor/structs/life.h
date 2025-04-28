#pragma once
#include "../structs/int_types.h"
#include <string>
#include <vector>

using namespace std;

namespace EvalEnum {
	enum EvalEnum
	{
		LCONST,
		GET,
		ACTOR_COLLIDER,
		TRIGGER_COLLIDER,
		HARD_COLLIDER,
		HIT_TO,
		HIT_BY,
		ANIM,
		END_ANIM,
		FRAME,
		END_FRAME,
		MODEL,
		MARK,
		NUM_TRACK,
		CHRONO,
		ROOM_CHRONO,
		DIST,
		COL_BY,
		IN_INVENTORY,
		ACTION,
		POSREL,
		KEYBOARD_INPUT,
		SPACE,
		CONTACT,
		ALPHA,
		BETA,
		GAMMA,
		IN_HAND,
		HITFORCE,
		CAMERA,
		RAND,
		FALLING,
		ROOM,
		LIFE,
		IS_FOUND,
		ROOMY,
		TEST_ZV_END_ANIM,
		MUSIC,
		GET_C,
		STAGE,
		THROWING,
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
		SET_MODEL,
		IF_EGAL,
		IF_DIFFERENT,
		IF_SUP_EGAL,
		IF_SUP,
		IF_INF_EGAL,
		IF_INF,
		GOTO,
		RETURN,
		ENDLIFE,
		SET_ANIM_REPEAT,
		SET_ANIM_MOVE,
		SET_TRACKMODE,
		HIT,
		MESSAGE,
		MESSAGE_VALUE,
		SET,
		INC,
		DEC,
		ADD,
		SUB,
		SET_LIFE_MODE,
		SWITCH,
		CASE,
		CASE_ELSE,
		CAMERA,
		START_CHRONO,
		MULTI_CASE,
		FOUND,
		SET_LIFE,
		DELETE_OBJ,
		TAKE,
		SET_IN_HAND,
		READ,
		READ_CD,
		SET_ANIM_SOUND,
		SPECIAL,
		DO_REAL_ZV,
		SOUND,
		SET_FLAGS,
		GAME_OVER,
		MANUAL_ROT,
		RND_FREQ,
		SET_MUSIC,
		SET_BETA,
		DO_ROT_ZV,
		CHANGE_ROOM,
		SET_INVENTORY_NAME,
		SET_INVENTORY_FLAG,
		SET_INVENTORY_LIFE,
		SET_CAMERA_TARGET,
		DROP,
		FIRE,
		TEST_COL,
		SET_INVENTORY_MODEL,
		SET_ALPHA,
		STOP_BETA,
		DO_MAX_ZV,
		PUT,
		SET_C,
		DO_NORMAL_ZV,
		DO_CARRE_ZV,
		SOUND_THEN,
		SET_LIGHT,
		SET_SHAKING,
		ALLOW_INVENTORY,
		SET_OBJ_WEIGHT,
		UP_COOR_Y,
		SPEED,
		PUT_AT,
		DEF_ZV,
		HIT_OBJECT,
		GET_HARD_CLIP,
		SET_ANGLE,
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
	EvalEnum::EvalEnum type;
	string typeStr;
	vector<parseArgType> arguments;
	bool needActor = false;
};

struct parseLifeInstruction {
	LifeEnum::LifeEnum type;
	string typeStr;
	vector<parseArgType> arguments;
	bool needActor = false;
};

typedef struct LifeExpr
{
	const parseLifeExpr* type = 0;
	s16 Actor = -1;
	s16 constVal = 0;
	vector<LifeExpr> arguments;
} LifeExpr;

typedef struct LifeInstruction
{
	const parseLifeInstruction* type = 0;
	vector<LifeExpr> arguments;
	s16 Actor = -1;
	int Goto = -1;
	int Position = -1; //used to map gotos to a given instruction
	int Size = -1;     //used to map gotos to a given instruction
} LifeInstruction;

typedef vector<LifeInstruction> LifeInstructions;
typedef vector<LifeInstruction*> LifeInstructionsP;
typedef struct LifeNode;

typedef struct LifeCase
{
	LifeInstruction* caseInstr;
	vector<LifeNode> instructs;
	bool isElse = false;
} LifeCase;

struct LifeNode
{
	//for simple of case
	LifeInstruction* instr = 0;
	//for if
	LifeInstructionsP ifConditions;
	vector<LifeNode> ifInstructs;
	vector<LifeNode> elseInstructs;
	//for case
	vector<LifeCase> cases;
};