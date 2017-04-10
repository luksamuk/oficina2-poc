-- player.lua
-- Sonic behaviour

local paused = false
local super  = false
local currentPlayer = PLAYER_ONE

local animator = nil

local speed       = {0.0, 0.0}
local topSpd      = {0.0, 0.0}
local maxSpd      = {0.0, 0.0}
local accel       = 0.046875
local airAccel    = 0.1875
local airDrag     = 0.96875
local airDragMinX = 0.0125
local airDragMinY = -4.0
local decel       = 0.5
local grav        = 0.21875
local jmpstr      = -6.5
local minjmp      = -4.0

local ground      = false
local direction   = 1.0

local ACT_NONE, ACT_JMP, ACT_SKID,
      ACT_LOOKUP, ACT_CROUCH, ACT_PUSH = 0, 1, 2, 3, 4, 5

local action = ACT_NONE

local rightBound  = 3840.0
local fakegroundY = 192.0

function clamp(val, min, max)
    if val < min then
        return min
    elseif val > max then
        return max
    else return val end
end

function defPlayerValues()
    if not super then
        -- Sonic
        accel       = 0.046875
        airAccel    = 0.1875
        airDragMinX = 0.96875
        airDragMinY = -4.0
        maxSpd[X]   = 12.0
        topSpd[X]   = 6.0
        decel       = 0.5
        grav        = 0.21875
        jmpstr      = -6.5
        minjmp      = -4.0
    else
        -- Super Sonic
        accel       = 0.1875
        airAccel    = 0.375
        topSpd[X]   = 10.0
        decel       = 1.0
        jmpstr      = -8.0
    end
end

function init()
    action    = ACT_NONE
    animator  = entity.getComponent("Animator")
    defPlayerValues()
    direction = 1.0
    entity.translate({128.0, fakegroundY, 0.0}, true)
end

function update(dt)
    -- Pause handle
    if common.btntap(PAD_START, currentPlayer) then
        paused = not paused
        render.animator.setRunning(animator, not paused)
    end

    if not paused then
        local lstk = common.lstick(currentPlayer)
        -- Acceleration
        if (math.abs(speed[X]) < topSpd[X])
            and ((action == ACT_NONE)
                 or (action == ACT_JMP)
                 or (action == ACT_SKID)) then
           speed[X] = speed[X] + (lstk[X] *
                        (if ground then accel else airAccel end))
        end

        -- X axis
        local currspd = speed[X]
        -- Default deceleration
        if ground and (lstk[X] == 0.0) then
            if (currspd > (-accel)) and (currspd < accel) then
                currspd = 0.0
            elseif currspd < 0.0 then
                currspd = currspd + accel
            elseif currspd > 0.0 then
                currspd = currspd - accel
            end
        end
        -- Skidding
        if ground then
            if (currspd > 0.0) and (lstk[X] < 0.0) then
                if(math.abs(currspd) > 1.8) then
                    render.animator.setAnimation(animator, "Skid")
                end
                action = ACT_SKID
                currspd = currspd - decel
            end
            if (currspd < 0.0) and (lstk[X] > 0.0) then
                if(math.abs(currspd) > 1.8) then
                    render.animator.setAnimation(animator, "Skid")
                end
                action = ACT_SKID
                currspd = currspd + decel
            end
            -- Reset skidding
            if (action == ACT_SKID)
                and (currspd > (-decel))
                and (currspd < decel) then
                action = ACT_NONE
                currspd = 0.0
            end
        end

        -- Left boundary limit
        
    end
end