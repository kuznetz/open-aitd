-- Loaded AFTER data/scripts/main.lua

function END_SEQUENCE()
  LOG("END_SEQUENCE");
  --CUTSCENE(0);
end

-- L_PLAYER_CLIMBING - climb fix
function life_552(obj)
  if (ANIM(obj) == 267) then
    if (END_ANIM(obj) == 1) then
      UP_COOR_Y()
      SET_ANIM_ALL_ONCE(obj, 268, 4)
      SET_LIFE(obj, Life.PLAYER_AFTER_CLIMBING)
      TEST_COL(obj, 1)
    else
      TEST_COL(obj, 0) --CLIMBING FIX
    end
  else
    SET_LIFE(obj, Life.PLAYER_NORMAL)
    ALLOW_INVENTORY(1)
    SET(Vars.PLAYER_HAS_CONTROL, 1)
    SET_TRACKMODE(obj, 1, -1)
    TEST_COL(obj, 1)
  end
  if (ALT_MODELS() == 0) then
    SET_ANIM_SOUND(obj, 22, 267, 3)
    SET_ANIM_SOUND(obj, 24, 267, 6)
  else
    SET_ANIM_SOUND(obj, 23, 267, 3)
    SET_ANIM_SOUND(obj, 25, 267, 6)
  end
end

-- L_E1R3_FIRST_AID_CABINET (animation fix)
function life_54(obj)
  SET_BETA(obj, 512, 120)
  DO_ROT_ZV(obj)
  if (BETA(obj) == 512) then
    if (IS_FOUND(GObj.FIRST_AID_CASE) == 0) then
      FOUND(GObj.FIRST_AID_CASE)
    else
      MESSAGE(100)
    end
    SET_LIFE(obj, Life.E1R3_FIRST_AID_CABINET_CLOSING)
    SET_ANIM_REPEAT(GObj.PLAYER, 4)
    SET_TRACKMODE(GObj.PLAYER, 1, -1)
    SET(Vars.PLAYER_HAS_CONTROL, 1)
    TEST_COL(GObj.PLAYER, 1)
  end
end

-- L_E1R3_FIRST_AID_CABINET_CLOSING
function life_55(obj)
  SET_BETA(obj, 256, 120)
  DO_ROT_ZV(obj)
  if (BETA(obj) == 256) then
    SET_LIFE(obj, -1)
    SET_FLAGS(obj, 0)
    SET(Vars.OPENING_CABINET_IN_E1R3, 0)
  end
end

-- L_E1R3_SCRIPT (animation fix)
function life_56(obj)
  sw_1 = HARD_COLLIDER(GObj.PLAYER)
  if sw_1 == 0 then
    if (GET(Vars.OPENING_CABINET_IN_E1R3) == 0) and ANIM(GObj.PLAYER) == 2 and (POSREL(obj, 1) == 4) and (END_ANIM(GObj.PLAYER) == 1) then
      LOG("Opening")
      SET(Vars.PLAYER_HAS_CONTROL, 0)
      SET_TRACKMODE(GObj.PLAYER, 0, -1)
      SET_ANIM_ONCE(GObj.PLAYER, 27, 4)
      SET_LIFE(GObj.E1R3_CABINET_DOOR, Life.E1R3_FIRST_AID_CABINET)
      SET_FLAGS(GObj.E1R3_CABINET_DOOR, 1)
      SOUND(4)
      SET(Vars.OPENING_CABINET_IN_E1R3, 1)
    end
  elseif (sw_1 == 1) or (sw_1 == 2) then
    if (ANIM(GObj.PLAYER) == 2) and (END_ANIM(GObj.PLAYER) == 1) then
      MESSAGE(704)
    end
  end
  if (GET(Vars.FOOTSTEP_SOUND_1) ~= 32) then
    SET(Vars.FOOTSTEP_SOUND_1, 32)
    SET(Vars.FOOTSTEP_SOUND_2, 33)
  end
end