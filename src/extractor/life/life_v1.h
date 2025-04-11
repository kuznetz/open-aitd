﻿#include "../structs/life.h"

inline const vector<parseLifeInstruction> LifeParams =
{
	{ LifeEnum::DO_MOVE, "DO_MOVE", { }, true},
	{ LifeEnum::ANIM_ONCE, "SET_ANIM_ONCE", { lifeConst, lifeConst }, true},
	{ LifeEnum::ANIM_ALL_ONCE, "SET_ANIM_ALL_ONCE", { lifeConst, lifeConst }, true},
	{ LifeEnum::SET_MODEL, "SET_MODEL", { lifeExpr }, true},
	{ LifeEnum::IF_EGAL, "IF_EGAL", { lifeExpr, lifeExpr, lifeGoto }},
	{ LifeEnum::IF_DIFFERENT, "IF_DIFFERENT", { lifeExpr, lifeExpr, lifeGoto }},
	{ LifeEnum::IF_SUP_EGAL, "IF_SUP_EGAL", { lifeExpr, lifeExpr, lifeGoto }},
	{ LifeEnum::IF_SUP, "IF_SUP", { lifeExpr, lifeExpr, lifeGoto }},
	{ LifeEnum::IF_INF_EGAL, "IF_INF_EGAL", { lifeExpr, lifeExpr, lifeGoto }},
	{ LifeEnum::IF_INF, "IF_INF", { lifeExpr, lifeExpr, lifeGoto }},
	{ LifeEnum::GOTO, "GOTO", { lifeGoto }},
	{ LifeEnum::RETURN, "RETURN", { }},
	{ LifeEnum::ENDLIFE, "ENDLIFE", { }},
	{ LifeEnum::SET_ANIM_REPEAT, "SET_ANIM_REPEAT", { lifeConst }, true},
	{ LifeEnum::SET_ANIM_MOVE, "SET_ANIM_MOVE", { lifeConst, lifeConst, lifeConst, lifeConst, lifeConst, lifeConst, lifeConst }, true},
	{ LifeEnum::SET_TRACKMODE, "SET_TRACKMODE", { lifeConst, lifeConst }, true},
	{ LifeEnum::HIT, "HIT", { lifeConst, lifeConst, lifeConst, lifeConst, lifeExpr, lifeConst }, true},
	{ LifeEnum::MESSAGE, "MESSAGE", { lifeConst }},
	{ LifeEnum::MESSAGE_VALUE, "MESSAGE_VALUE", { }}, //NOT_USE
	{ LifeEnum::SET, "SET", { lifeConst, lifeExpr }},
	{ LifeEnum::INC, "INC", { lifeConst }},
	{ LifeEnum::DEC, "DEC", { lifeConst }},
	{ LifeEnum::ADD, "ADD", { lifeConst, lifeExpr }},
	{ LifeEnum::SUB, "SUB", { lifeConst, lifeExpr }},
	{ LifeEnum::SET_LIFE_MODE, "SET_LIFE_MODE", { lifeConst }, true},
	{ LifeEnum::SWITCH, "SWITCH", { lifeExpr }},
	{ LifeEnum::CASE, "CASE", { lifeConst, lifeGoto }},
	{ LifeEnum::CAMERA, "CAMERA", { }}, //???
	{ LifeEnum::START_CHRONO, "START_CHRONO", { }, true},
	{ LifeEnum::MULTI_CASE, "MULTI_CASE", {}}, //Custom parser
	{ LifeEnum::FOUND, "FOUND", { lifeConst }},
	{ LifeEnum::SET_LIFE, "SET_LIFE", { lifeConst }, true},
	{ LifeEnum::DELETE_OBJ, "DELETE_OBJ", { lifeConst }},
	{ LifeEnum::TAKE, "TAKE", { lifeConst }},
	{ LifeEnum::IN_HAND, "IN_HAND", { lifeConst }},
	{ LifeEnum::READ, "READ", { lifeConst, lifeConst }}, //3rd lifeConst in cd version
	{ LifeEnum::SET_ANIM_SOUND, "SET_ANIM_SOUND", { lifeExpr, lifeConst, lifeConst }, true},
	{ LifeEnum::SPECIAL, "SPECIAL", { lifeConst }},
	{ LifeEnum::DO_REAL_ZV, "DO_REAL_ZV", { }, true},
	{ LifeEnum::SOUND, "SOUND", { lifeExpr }},
	{ LifeEnum::SET_FLAGS, "SET_FLAGS", { lifeConst }, true},
	{ LifeEnum::GAME_OVER, "GAME_OVER", { }},
	{ LifeEnum::MANUAL_ROT, "MANUAL_ROT", { }, true},
	{ LifeEnum::RND_FREQ, "RND_FREQ", { lifeConst }}, //Randomize sound frequency
	{ LifeEnum::SET_MUSIC, "SET_MUSIC", { lifeConst }},
	{ LifeEnum::SET_BETA, "SET_BETA", { lifeConst, lifeConst }, true},
	{ LifeEnum::DO_ROT_ZV, "DO_ROT_ZV", { }, true},
	{ LifeEnum::CHANGE_ROOM, "CHANGE_ROOM", { lifeConst, lifeConst, lifeConst, lifeConst, lifeConst }, true},
	{ LifeEnum::SET_INVENTORY_NAME, "SET_INVENTORY_NAME", { lifeConst }, true},
	{ LifeEnum::SET_INVENTORY_FLAG, "SET_INVENTORY_FLAG", { lifeConst }, true},
	{ LifeEnum::SET_INVENTORY_LIFE, "SET_INVENTORY_LIFE", { }, true},
	{ LifeEnum::SET_CAMERA_TARGET, "SET_CAMERA_TARGET", { lifeConst }},
	{ LifeEnum::DROP, "DROP", { lifeExpr, lifeConst }},
	{ LifeEnum::FIRE, "FIRE", { lifeConst, lifeConst, lifeConst, lifeConst, lifeConst, lifeConst }, true},
	{ LifeEnum::TEST_COL, "TEST_COL", { lifeConst }, true},
	{ LifeEnum::SET_INVENTORY_MODEL, "SET_INVENTORY_MODEL", { lifeConst }, true},
	{ LifeEnum::SET_ALPHA, "SET_ALPHA", { lifeConst, lifeConst }, true},
	{ LifeEnum::STOP_BETA, "STOP_BETA", { }},
	{ LifeEnum::DO_MAX_ZV, "DO_MAX_ZV", { }, true},
	{ LifeEnum::PUT, "PUT", { lifeConst, lifeConst, lifeConst, lifeConst, lifeConst, lifeConst, lifeConst, lifeConst, lifeConst }},
	{ LifeEnum::SET_C, "SET_C", { lifeConst, lifeExpr }},
	{ LifeEnum::DO_NORMAL_ZV, "DO_NORMAL_ZV", { }},
	{ LifeEnum::DO_CARRE_ZV, "DO_CARRE_ZV", { }, true},
	{ LifeEnum::SOUND_THEN, "SOUND_THEN", { lifeExpr, lifeExpr }},
	{ LifeEnum::SET_LIGHT, "SET_LIGHT", { lifeConst }},
	{ LifeEnum::SET_SHAKING, "SET_SHAKING", { lifeConst }},
	{ LifeEnum::ALLOW_INVENTORY, "ALLOW_INVENTORY", { lifeConst }},
	{ LifeEnum::SET_OBJ_WEIGHT, "SET_OBJ_WEIGHT", { lifeConst }, true},
	{ LifeEnum::UP_COOR_Y, "UP_COOR_Y", { }},
	{ LifeEnum::SPEED, "SPEED", { }},
	{ LifeEnum::PUT_AT, "PUT_AT", { lifeConst, lifeConst }},
	{ LifeEnum::DEF_ZV, "DEF_ZV", { lifeConst, lifeConst, lifeConst, lifeConst, lifeConst, lifeConst }, true},
	{ LifeEnum::HIT_OBJECT, "HIT_OBJECT", { lifeConst, lifeConst }, true},
	{ LifeEnum::GET_HARD_CLIP, "GET_HARD_CLIP", { }},
	{ LifeEnum::SET_ANGLE, "SET_ANGLE", { lifeConst, lifeConst, lifeConst }, true},
	{ LifeEnum::REP_SOUND, "REP_SOUND", { lifeExpr, lifeConst }},
	{ LifeEnum::THROW, "THROW", { lifeConst, lifeConst, lifeConst, lifeConst, lifeConst, lifeConst, lifeConst }, true},
	{ LifeEnum::WATER, "WATER", { lifeConst }},
	{ LifeEnum::PICTURE, "PICTURE", { lifeConst, lifeConst, lifeConst }},
	{ LifeEnum::STOP_SOUND, "STOP_SOUND", { }},
	{ LifeEnum::NEXT_MUSIC, "NEXT_MUSIC", { lifeConst }},
	{ LifeEnum::FADE_MUSIC, "FADE_MUSIC", { lifeConst }},
	{ LifeEnum::STOP_HIT_OBJECT, "STOP_HIT_OBJECT", { }},
	{ LifeEnum::COPY_ANGLE, "COPY_ANGLE", { lifeConst }},
	{ LifeEnum::END_SEQUENCE, "END_SEQUENCE", { }},
	{ LifeEnum::SOUND_THEN_REPEAT, "SOUND_THEN_REPEAT", { }},
	{ LifeEnum::WAIT_GAME_OVER, "WAIT_GAME_OVER", { }},
	{ LifeEnum::GET_MATRICE, "GET_MATRICE", { }},
	{ LifeEnum::STAGE_LIFE, "STAGE_LIFE", { lifeConst }},
	{ LifeEnum::CONTINUE_TRACK, "CONTINUE_TRACK", { }},
	{ LifeEnum::ANIM_RESET, "ANIM_RESET", { lifeConst, lifeConst }},
	{ LifeEnum::RESET_MOVE_MANUAL, "RESET_MOVE_MANUAL", { }},
	{ LifeEnum::PLUIE, "PLUIE", { }},
	{ LifeEnum::ANIM_HYBRIDE_ONCE, "ANIM_HYBRIDE_ONCE", { lifeConst, lifeConst }},
	{ LifeEnum::ANIM_HYBRIDE_REPEAT, "ANIM_HYBRIDE_REPEAT", { lifeConst, lifeConst }},
	{ LifeEnum::CALL_INVENTORY, "CALL_INVENTORY", { }},
	{ LifeEnum::BODY_RESET, "BODY_RESET", { lifeExpr, lifeExpr }},
	{ LifeEnum::DEL_INVENTORY, "DEL_INVENTORY", { }},
	{ LifeEnum::SET_INVENTORY, "SET_INVENTORY", { lifeConst }},
	{ LifeEnum::PLAY_SEQUENCE, "PLAY_SEQUENCE", { lifeConst }},
	{ LifeEnum::_2D_ANIM_SOUND, "_2D_ANIM_SOUND", { }},
	{ LifeEnum::SET_GROUND, "SET_GROUND", { lifeConst }},
	{ LifeEnum::PROTECT, "PROTECT", { }},
	{ LifeEnum::DEF_ABS_ZV, "DEF_ABS_ZV", { lifeConst, lifeConst, lifeConst, lifeConst, lifeConst, lifeConst }, true },
	{ LifeEnum::DEF_SEQUENCE_SOUND, "DEF_SEQUENCE_SOUND", { lifeConst }},
	{ LifeEnum::READ_ON_PICTURE, "READ_ON_PICTURE", { lifeConst, lifeConst, lifeConst, lifeConst, lifeConst, lifeConst, lifeConst, lifeConst }},
	{ LifeEnum::FIRE_UP_DOWN, "FIRE_UP_DOWN", { lifeExpr, lifeConst, lifeConst, lifeConst, lifeConst, lifeConst, lifeConst, lifeExpr }},
	{ LifeEnum::DO_ROT_CLUT, "DO_ROT_CLUT", { lifeConst, lifeConst, lifeConst }, true },
	{ LifeEnum::STOP_CLUT, "STOP_CLUT", { }, true },
};

// Start by 0
inline const vector<parseLifeExpr> LifeExprParams =
{
	{ EvalEnum::GET, "GET", { lifeConst }},
	{ EvalEnum::ACTOR_COLLIDER, "ACTOR_COLLIDER", { }, true },
	{ EvalEnum::TRIGGER_COLLIDER, "TRIGGER_COLLIDER", { }, true },
	{ EvalEnum::HARD_COLLIDER, "HARD_COLLIDER", { }, true },
	{ EvalEnum::HIT, "HIT", { }, true },
	{ EvalEnum::HIT_BY, "HIT_BY", { }, true},
	{ EvalEnum::ANIM, "ANIM", { }, true },
	{ EvalEnum::END_ANIM, "END_ANIM", { }, true },
	{ EvalEnum::FRAME, "FRAME", { }, true},
	{ EvalEnum::END_FRAME, "END_FRAME", { }},
	{ EvalEnum::MODEL, "MODEL", { }, true },
	{ EvalEnum::MARK, "MARK", { }, true },
	{ EvalEnum::NUM_TRACK, "NUM_TRACK", { }, true },
	{ EvalEnum::CHRONO, "CHRONO", { }, true },
	{ EvalEnum::ROOM_CHRONO, "ROOM_CHRONO", { }},
	{ EvalEnum::DIST, "DIST", { lifeConst }},
	{ EvalEnum::COL_BY, "COL_BY", { }, true },
	{ EvalEnum::IN_INVENTORY, "IN_INVENTORY", { lifeExpr }},
	{ EvalEnum::ACTION, "ACTION", { }},
	{ EvalEnum::POSREL, "POSREL", { lifeConst }},
	{ EvalEnum::KEYBOARD_INPUT, "KEYBOARD_INPUT", { }},
	{ EvalEnum::SPACE, "SPACE", { }},
	{ EvalEnum::CONTACT, "CONTACT", { }},
	{ EvalEnum::ALPHA, "ALPHA", { }, true },
	{ EvalEnum::BETA, "BETA", { }, true },
	{ EvalEnum::GAMMA, "GAMMA", { }, true },
	{ EvalEnum::INHAND, "INHAND", { }},
	{ EvalEnum::HITFORCE, "HITFORCE", { }},
	{ EvalEnum::CAMERA, "CAMERA", { }},
	{ EvalEnum::RAND, "RAND", { lifeConst }},
	{ EvalEnum::FALLING, "FALLING", { }},
	{ EvalEnum::ROOM, "ROOM", { }, true },
	{ EvalEnum::LIFE, "LIFE", { }, true },
	{ EvalEnum::IS_FOUND, "IS_FOUND", { lifeConst }},
	{ EvalEnum::ROOMY, "ROOMY", { }, true },
	{ EvalEnum::TEST_ZV_END_ANIM, "TEST_ZV_END_ANIM", { lifeConst, lifeConst }},
	{ EvalEnum::MUSIC, "MUSIC", { }},
	{ EvalEnum::GET_C, "GET_C", { lifeConst }},
	{ EvalEnum::STAGE, "STAGE", { }, true },
	{ EvalEnum::THROWING, "THROWING", { lifeConst }}
};

inline const vector<LifeEnum::LifeEnum> LifeTable_v1 = {
	LifeEnum::DO_MOVE,
	LifeEnum::ANIM_ONCE,
	LifeEnum::ANIM_ALL_ONCE,
	LifeEnum::SET_MODEL,
	LifeEnum::IF_EGAL,
	LifeEnum::IF_DIFFERENT,
	LifeEnum::IF_SUP_EGAL,
	LifeEnum::IF_SUP,
	LifeEnum::IF_INF_EGAL,
	LifeEnum::IF_INF,
	LifeEnum::GOTO,
	LifeEnum::RETURN,
	LifeEnum::ENDLIFE,
	LifeEnum::SET_ANIM_REPEAT,
	LifeEnum::SET_ANIM_MOVE,
	LifeEnum::SET_TRACKMODE,
	LifeEnum::HIT,
	LifeEnum::MESSAGE,
	LifeEnum::MESSAGE_VALUE,
	LifeEnum::SET,
	LifeEnum::INC,
	LifeEnum::DEC,
	LifeEnum::ADD,
	LifeEnum::SUB,
	LifeEnum::SET_LIFE_MODE,
	LifeEnum::SWITCH,
	LifeEnum::CASE,
	LifeEnum::CAMERA,
	LifeEnum::START_CHRONO,
	LifeEnum::MULTI_CASE,
	LifeEnum::FOUND,
	LifeEnum::SET_LIFE,
	LifeEnum::DELETE_OBJ,
	LifeEnum::TAKE,
	LifeEnum::IN_HAND,
	LifeEnum::READ,
	LifeEnum::SET_ANIM_SOUND,
	LifeEnum::SPECIAL,
	LifeEnum::DO_REAL_ZV,
	LifeEnum::SOUND,
	LifeEnum::SET_FLAGS,
	LifeEnum::GAME_OVER,
	LifeEnum::MANUAL_ROT,
	LifeEnum::RND_FREQ,
	LifeEnum::SET_MUSIC,
	LifeEnum::SET_BETA,
	LifeEnum::DO_ROT_ZV,
	LifeEnum::CHANGE_ROOM,
	LifeEnum::SET_INVENTORY_NAME,
	LifeEnum::SET_INVENTORY_FLAG,
	LifeEnum::SET_INVENTORY_LIFE,
	LifeEnum::SET_CAMERA_TARGET,
	LifeEnum::DROP,
	LifeEnum::FIRE,
	LifeEnum::TEST_COL,
	LifeEnum::SET_INVENTORY_MODEL,
	LifeEnum::SET_ALPHA,
	LifeEnum::STOP_BETA,
	LifeEnum::DO_MAX_ZV,
	LifeEnum::PUT,
	LifeEnum::SET_C,
	LifeEnum::DO_NORMAL_ZV,
	LifeEnum::DO_CARRE_ZV,
	LifeEnum::SOUND_THEN,
	LifeEnum::SET_LIGHT,
	LifeEnum::SET_SHAKING,
	LifeEnum::ALLOW_INVENTORY,
	LifeEnum::SET_OBJ_WEIGHT,
	LifeEnum::UP_COOR_Y,
	LifeEnum::SPEED,
	LifeEnum::PUT_AT,
	LifeEnum::DEF_ZV,
	LifeEnum::HIT_OBJECT,
	LifeEnum::GET_HARD_CLIP,
	LifeEnum::SET_ANGLE,
	LifeEnum::REP_SOUND,
	LifeEnum::THROW,
	LifeEnum::WATER,
	LifeEnum::PICTURE,
	LifeEnum::STOP_SOUND,
	LifeEnum::NEXT_MUSIC,
	LifeEnum::FADE_MUSIC,
	LifeEnum::STOP_HIT_OBJECT,
	LifeEnum::COPY_ANGLE,
	LifeEnum::END_SEQUENCE,
	LifeEnum::SOUND_THEN_REPEAT,
	LifeEnum::WAIT_GAME_OVER,
	LifeEnum::GET_MATRICE,
	LifeEnum::STAGE_LIFE,
	LifeEnum::CONTINUE_TRACK,
	LifeEnum::ANIM_RESET,
	LifeEnum::RESET_MOVE_MANUAL,
	LifeEnum::PLUIE,
	LifeEnum::ANIM_HYBRIDE_ONCE,
	LifeEnum::ANIM_HYBRIDE_REPEAT,
	LifeEnum::CALL_INVENTORY,
	LifeEnum::BODY_RESET,
	LifeEnum::DEL_INVENTORY,
	LifeEnum::SET_INVENTORY,
	LifeEnum::PLAY_SEQUENCE,
	LifeEnum::_2D_ANIM_SOUND,
	LifeEnum::SET_GROUND,
	LifeEnum::PROTECT,
	LifeEnum::DEF_ABS_ZV,
	LifeEnum::DEF_SEQUENCE_SOUND,
	LifeEnum::READ_ON_PICTURE,
	LifeEnum::FIRE_UP_DOWN,
	LifeEnum::DO_ROT_CLUT,
	LifeEnum::STOP_CLUT,
};

inline const vector<EvalEnum::EvalEnum> ExprTable_v1 = {
	EvalEnum::GET,
	EvalEnum::ACTOR_COLLIDER,
	EvalEnum::TRIGGER_COLLIDER,
	EvalEnum::HARD_COLLIDER,
	EvalEnum::HIT,
	EvalEnum::HIT_BY,
	EvalEnum::ANIM,
	EvalEnum::END_ANIM,
	EvalEnum::FRAME,
	EvalEnum::END_FRAME,
	EvalEnum::MODEL,
	EvalEnum::MARK,
	EvalEnum::NUM_TRACK,
	EvalEnum::CHRONO,
	EvalEnum::ROOM_CHRONO,
	EvalEnum::DIST,
	EvalEnum::COL_BY,
	EvalEnum::IN_INVENTORY,
	EvalEnum::ACTION,
	EvalEnum::POSREL,
	EvalEnum::KEYBOARD_INPUT,
	EvalEnum::SPACE,
	EvalEnum::CONTACT,
	EvalEnum::ALPHA,
	EvalEnum::BETA,
	EvalEnum::GAMMA,
	EvalEnum::INHAND,
	EvalEnum::HITFORCE,
	EvalEnum::CAMERA,
	EvalEnum::RAND,
	EvalEnum::FALLING,
	EvalEnum::ROOM,
	EvalEnum::LIFE,
	EvalEnum::IS_FOUND,
	EvalEnum::ROOMY,
	EvalEnum::TEST_ZV_END_ANIM,
	EvalEnum::MUSIC,
	EvalEnum::GET_C,
	EvalEnum::STAGE,
	EvalEnum::THROWING,
};