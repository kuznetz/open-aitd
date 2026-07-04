-- L_PLAYER_NORMAL
function life_549(obj)
  if (var_21 > 0) then
    if (SPACE() == 0) then
      DO_MOVE(obj)
      if (var_0 == 1) then
        ALLOW_INVENTORY(1)
        sw_4 = MODEL(obj)
        if sw_4 == 12 then
          SET_ANIM_MOVE(obj, 4, 254, 255, 4, 256, 257, 258)
          RND_FREQ(60)
          SET_ANIM_SOUND(obj, var_4, 254, 2)
          SET_ANIM_SOUND(obj, var_5, 254, 5)
          SET_ANIM_SOUND(obj, var_5, 256, 1)
          SET_ANIM_SOUND(obj, var_4, 256, 3)
          SET_ANIM_SOUND(obj, var_5, 255, 1)
          SET_ANIM_SOUND(obj, var_4, 255, 3)
          RND_FREQ(0)
          if (ANIM(obj) == 256) then
            var_24 = 1
          else
            var_24 = 0
          end
        elseif sw_4 == 15 then
          SET_ANIM_MOVE(obj, 278, 279, 280, 278, 281, 283, 284)
          RND_FREQ(60)
          SET_ANIM_SOUND(obj, var_4, 279, 2)
          SET_ANIM_SOUND(obj, var_5, 279, 5)
          SET_ANIM_SOUND(obj, var_5, 281, 1)
          SET_ANIM_SOUND(obj, var_4, 281, 3)
          SET_ANIM_SOUND(obj, var_5, 280, 1)
          SET_ANIM_SOUND(obj, var_4, 280, 3)
          RND_FREQ(0)
          if (ANIM(obj) == 281) then
            var_24 = 1
          else
            var_24 = 0
          end
        elseif sw_4 == 193 then
          SET_ANIM_MOVE(obj, 278, 279, 286, 278, 281, 283, 284)
          RND_FREQ(60)
          SET_ANIM_SOUND(obj, var_4, 279, 2)
          SET_ANIM_SOUND(obj, var_5, 279, 5)
          SET_ANIM_SOUND(obj, var_5, 281, 1)
          SET_ANIM_SOUND(obj, var_4, 281, 3)
          SET_ANIM_SOUND(obj, var_5, 286, 1)
          SET_ANIM_SOUND(obj, var_4, 286, 3)
          RND_FREQ(0)
          if (ANIM(obj) == 281) then
            var_24 = 1
          else
            var_24 = 0
          end
        elseif sw_4 == 11 then
          SET_ANIM_MOVE(obj, 287, 288, 289, 287, 290, 292, 293)
          RND_FREQ(60)
          SET_ANIM_SOUND(obj, var_4, 288, 2)
          SET_ANIM_SOUND(obj, var_5, 288, 5)
          SET_ANIM_SOUND(obj, var_5, 290, 1)
          SET_ANIM_SOUND(obj, var_4, 290, 3)
          SET_ANIM_SOUND(obj, var_5, 289, 1)
          SET_ANIM_SOUND(obj, var_4, 289, 3)
          RND_FREQ(0)
          if (ANIM(obj) == 290) then
            var_24 = 1
          else
            var_24 = 0
          end
        elseif (sw_4 == 44) or (sw_4 == 45) or (sw_4 == 118) then
          SET_ANIM_MOVE(obj, 4, 295, 296, 4, 256, 257, 258)
          RND_FREQ(60)
          SET_ANIM_SOUND(obj, var_4, 295, 2)
          SET_ANIM_SOUND(obj, var_5, 295, 5)
          SET_ANIM_SOUND(obj, var_5, 256, 1)
          SET_ANIM_SOUND(obj, var_4, 256, 3)
          SET_ANIM_SOUND(obj, var_5, 296, 1)
          SET_ANIM_SOUND(obj, var_4, 296, 3)
          RND_FREQ(0)
          if (ANIM(obj) == 256) then
            var_24 = 1
          else
            var_24 = 0
          end
        elseif (sw_4 == 31) or (sw_4 == 30) then
          SET_ANIM_MOVE(obj, 298, 299, 300, 298, 256, 302, 303)
          RND_FREQ(60)
          SET_ANIM_SOUND(obj, var_4, 299, 2)
          SET_ANIM_SOUND(obj, var_5, 299, 5)
          SET_ANIM_SOUND(obj, var_5, 281, 1)
          SET_ANIM_SOUND(obj, var_4, 281, 3)
          SET_ANIM_SOUND(obj, var_5, 300, 1)
          SET_ANIM_SOUND(obj, var_4, 300, 3)
          RND_FREQ(0)
          if (ANIM(obj) == 256) then
            var_24 = 1
          else
            var_24 = 0
          end
        end
      else
        ALLOW_INVENTORY(0)
      end
    end
    if (MODEL(obj) == Model.PLAYER_NORMAL) and (var_1 == 1) then
      if (ANIM(obj) ~= 5) then
        var_1 = 0
      else
        if (END_ANIM(obj) == 1) then
          var_1 = 0
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
        var_0 = 1
        SET_TRACKMODE(obj, 1, -1)
      end
    elseif sw_2 == 270 then      
      if (FRAME(obj) >= 7) then
        --LOG("JUMP >= 7")
        SET_FLAGS(obj, 321)
      else
        --LOG("JUMP < 7")
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
        var_24 = 0
      end
    elseif sw_2 == 11 then
      SET_ANIM_SOUND(obj, 65, 11, 3)
    end
    if (FALLING(obj) == 1) then
      if (var_168 == 1) then
        var_0 = 0
        SET_TRACKMODE(obj, 0, -1)
        var_29 = 1
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
      var_0 = 0
      ALLOW_INVENTORY(0)
      SET_LIFE(obj, Life.PLAYER_FALLING)
      if (ROOMY(obj) < -5000) then
        var_206 = 3
      else
        if (ROOMY(obj) < -3000) then
          var_206 = 2
        else
          var_206 = 1
        end
      end
    end
    if (HARD_COLLIDER(obj) == 255) and (ANIM(obj) ~= 270) and (var_24 == 0) and (TEST_ZV_END_ANIM(obj, 267, -2000) == 1) then
      --CLIMBING FIX
      var_0 = 0
      ALLOW_INVENTORY(0)
      SET_LIFE(obj, Life.PLAYER_CLIMBING)
      TEST_COL(obj, 0)
      SET_ANIM_ALL_ONCE(obj, 267, 268)
    end
    if (HIT_BY(obj) ~= -1) then
      SPECIAL(1)
      SET_LIFE(obj, Life.PLAYER_HIT_BY_SOMETHING)
    end
    if (var_10 == 1) and (END_ANIM(obj) == 1) then
      var_0 = 1
      var_10 = 0
      ALLOW_INVENTORY(1)
    end
  else
    SET_LIFE(obj, Life.DEATH_MANAGER)
  end
end
