#include "structs/int_types.h"
#include "pak/pak.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>

using namespace std;

typedef enum LifeOpParam {
	var,
	cvar,
	animation,
	sound,
	model,
	actor
} LifeOpParam;

typedef struct LifeOp {
	u8 code;
	string name;
	vector<u8> args;
} LifeOp;

const vector<LifeOp> lifeOps = { 
	{ 0, "DO_MOVE", {} },
	{ 0, "ANIM_ONCE", { animation, animation } },
	{ 0, "ANIM_ALL_ONCE", { animation, animation } }
};

void extractLife(string fname, string outDir)
{
	PakFile pak(fname);
	//for (int i = 0; i < 0; i++) {
	//
	//}
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