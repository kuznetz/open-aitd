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

function END_SEQUENCE()
  LOG("END_SEQUENCE");
end