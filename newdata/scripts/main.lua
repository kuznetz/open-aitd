-- L_PLAYER_NORMAL
function life_549(obj)
  if (GET(Vars.PLAYER_HITPOINTS) > 0) then
    if (SPACE() == 0) then
      DO_MOVE(obj)
      if (GET(Vars.PLAYER_HAS_CONTROL) == 1) then
        ALLOW_INVENTORY(1)
        sw_4 = MODEL(obj)
        if sw_4 == 12 then
          SET_ANIM_MOVE(obj, 4, 254, 255, 4, 256, 257, 258)
          RND_FREQ(60)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 254, 2)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 254, 5)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 256, 1)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 256, 3)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 255, 1)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 255, 3)
          RND_FREQ(0)
          if (ANIM(obj) == 256) then
            SET(Vars.PLAYER_MOVING_BACKWARDS, 1)
          else
            SET(Vars.PLAYER_MOVING_BACKWARDS, 0)
          end
        elseif sw_4 == 15 then
          SET_ANIM_MOVE(obj, 278, 279, 280, 278, 281, 283, 284)
          RND_FREQ(60)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 279, 2)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 279, 5)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 281, 1)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 281, 3)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 280, 1)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 280, 3)
          RND_FREQ(0)
          if (ANIM(obj) == 281) then
            SET(Vars.PLAYER_MOVING_BACKWARDS, 1)
          else
            SET(Vars.PLAYER_MOVING_BACKWARDS, 0)
          end
        elseif sw_4 == 193 then
          SET_ANIM_MOVE(obj, 278, 279, 286, 278, 281, 283, 284)
          RND_FREQ(60)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 279, 2)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 279, 5)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 281, 1)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 281, 3)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 286, 1)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 286, 3)
          RND_FREQ(0)
          if (ANIM(obj) == 281) then
            SET(Vars.PLAYER_MOVING_BACKWARDS, 1)
          else
            SET(Vars.PLAYER_MOVING_BACKWARDS, 0)
          end
        elseif sw_4 == 11 then
          SET_ANIM_MOVE(obj, 287, 288, 289, 287, 290, 292, 293)
          RND_FREQ(60)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 288, 2)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 288, 5)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 290, 1)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 290, 3)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 289, 1)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 289, 3)
          RND_FREQ(0)
          if (ANIM(obj) == 290) then
            SET(Vars.PLAYER_MOVING_BACKWARDS, 1)
          else
            SET(Vars.PLAYER_MOVING_BACKWARDS, 0)
          end
        elseif (sw_4 == 44) or (sw_4 == 45) or (sw_4 == 118) then
          SET_ANIM_MOVE(obj, 4, 295, 296, 4, 256, 257, 258)
          RND_FREQ(60)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 295, 2)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 295, 5)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 256, 1)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 256, 3)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 296, 1)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 296, 3)
          RND_FREQ(0)
          if (ANIM(obj) == 256) then
            SET(Vars.PLAYER_MOVING_BACKWARDS, 1)
          else
            SET(Vars.PLAYER_MOVING_BACKWARDS, 0)
          end
        elseif (sw_4 == 31) or (sw_4 == 30) then
          SET_ANIM_MOVE(obj, 298, 299, 300, 298, 256, 302, 303)
          RND_FREQ(60)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 299, 2)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 299, 5)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 281, 1)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 281, 3)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_2), 300, 1)
          SET_ANIM_SOUND(obj, GET(Vars.FOOTSTEP_SOUND_1), 300, 3)
          RND_FREQ(0)
          if (ANIM(obj) == 256) then
            SET(Vars.PLAYER_MOVING_BACKWARDS, 1)
          else
            SET(Vars.PLAYER_MOVING_BACKWARDS, 0)
          end
        end
      else
        ALLOW_INVENTORY(0)
      end
    end
    if (MODEL(obj) == Model.PLAYER_NORMAL) and (GET(Vars.PLAYER_PUSHING_SOMETHING) == 1) then
      if (ANIM(obj) ~= 5) then
        SET(Vars.PLAYER_PUSHING_SOMETHING, 0)
      else
        if (END_ANIM(obj) == 1) then
          SET(Vars.PLAYER_PUSHING_SOMETHING, 0)
        end
      end
      SET_ANIM_SOUND(obj, 5, 5, 1)
    end
    sw_2 = ANIM(obj)
    if sw_2 == 68 then
      SET_ANIM_SOUND(obj, 43, 68, 3)
      SET_ANIM_SOUND(obj, 43, 68, 6)
      SET_ANIM_SOUND(obj, 94, 68, 11)
      SET_ANIM_SOUND(obj, 64, 68, 13)
      SET_ANIM_SOUND(obj, 97, 68, 16)
      if (END_ANIM(obj) == 1) then
        SET(Vars.PLAYER_HAS_CONTROL, 1)
        SET_TRACKMODE(obj, 1, -1)
      end
    elseif sw_2 == 270 then
      if (FRAME(obj) >= 7) then
        --LOG("JUMP >= 7")
        SET_FLAGS(obj, 321)
      else
        if (END_ANIM(obj) == 0) then
          SET_FLAGS(GObj.PLAYER, 65)
        end
      end
      if (GET_C(8) == 0) then
        SET_ANIM_SOUND(obj, 22, 270, 2)
      else
        SET_ANIM_SOUND(obj, 23, 270, 2)
      end
    elseif sw_2 == 261 then
      SET_LIFE(obj, Life.DEATH_MANAGER)
    elseif (sw_2 == 237) or (sw_2 == 282) or (sw_2 == 291) or (sw_2 == 301) then
      if (END_ANIM(obj) == 1) then
        SET(Vars.PLAYER_MOVING_BACKWARDS, 0)
      end
    elseif sw_2 == 11 then
      SET_ANIM_SOUND(obj, 65, 11, 3)
    end
    if (FALLING(obj) == 1) then
      if (GET(Vars.PLAYER_INSIDE_TRIGGER_1_E5R0_OR_TRIGGER_0_E5R6) == 1) then
        SET(Vars.PLAYER_HAS_CONTROL, 0)
        SET_TRACKMODE(obj, 0, -1)
        SET(Vars.TYPE_OF_DEATH, 1)
        SET_LIFE(obj, Life.DEATH_MANAGER)
        if (GET_C(8) == 0) then
          SOUND(40)
        else
          SOUND(41)
        end
        ALLOW_INVENTORY(0)
        SET_FLAGS(obj, 65)
        do return end
      end
      SET_ANIM_REPEAT(obj, 269)
      SET(Vars.PLAYER_HAS_CONTROL, 0)
      ALLOW_INVENTORY(0)
      SET_LIFE(obj, Life.PLAYER_FALLING)
      if (ROOMY(obj) < -5000) then
        SET(Vars.FALL_HEIGHT, 3)
      else
        if (ROOMY(obj) < -3000) then
          SET(Vars.FALL_HEIGHT, 2)
        else
          SET(Vars.FALL_HEIGHT, 1)
        end
      end
    end
    if (HARD_COLLIDER(obj) == 255) and (ANIM(obj) ~= 270) and (GET(Vars.PLAYER_MOVING_BACKWARDS) == 0) and (TEST_ZV_END_ANIM(obj, 267, -2000) == 1) then
      --CLIMBING FIX
      SET(Vars.PLAYER_HAS_CONTROL, 0)
      ALLOW_INVENTORY(0)
      SET_LIFE(obj, Life.PLAYER_CLIMBING)
      TEST_COL(obj, 0)
      SET_ANIM_ALL_ONCE(obj, 267, 268)
    end
    if (HIT_BY(obj) ~= -1) then
      SPECIAL(1)
      SET_LIFE(obj, Life.PLAYER_HIT_BY_SOMETHING)
    end
    if (GET(Vars.THROWING_SOMETHING_USING_JUG) == 1) and (END_ANIM(obj) == 1) then
      SET(Vars.PLAYER_HAS_CONTROL, 1)
      SET(Vars.THROWING_SOMETHING_USING_JUG, 0)
      ALLOW_INVENTORY(1)
    end
  else
    SET_LIFE(obj, Life.DEATH_MANAGER)
  end
end

-- L_E6_SCRIPT (temporary simple light)
function life_509(obj)
  if (ROOM(GObj.PLAYER) < 6) then
    LOG("LIGHTMAN ACT")
    if (GET(Vars.NOT_IN_THE_DARK_IN_E5_E6) == 1) then
      --SET_LIGHT(0)
      SET(Vars.NOT_IN_THE_DARK_IN_E5_E6, 0)
      if (GET_C(12) == 0) then
        MESSAGE(105)
      end
    end
    if (STAGE(GObj.OIL_LAMP) == 6) then
      if (ROOM(GObj.OIL_LAMP) < 6) and (GET(Vars.OIL_LAMP_LIT) == 1) then
        --SET_C(13, 13)
        --LOG("lamp on floor")
        SET_LIGHT(1)
      end
    else
      if (IN_HAND(obj) == 13) then
        if (GET(Vars.OIL_LAMP_LIT) == 1) then
          --SET_C(13, 1)
          --LOG("lamp in hand")
          SET_LIGHT(1)
        end
      else
        --SET_C(13, -1)
        --LOG("no lamp")
        SET_LIGHT(0)
      end
    end
  else
    if (GET(Vars.NOT_IN_THE_DARK_IN_E5_E6) == 0) then
      SET(Vars.NOT_IN_THE_DARK_IN_E5_E6, 1)
      SET_LIGHT(1)
    end
  end
end

function END_SEQUENCE()
  LOG("END_SEQUENCE");
end