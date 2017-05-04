-- Constants
local gravity   = 0.8165
local accel     = 0.166
local decel     = 0.3
local jmpStg    = -13.0
local minJmp    = -6.5
local maxSpd    = 0.0
local defMaxSpd = 4.0
local runMaxSpd = 8.5
local hitboxRad = {16.0, 32.0}

-- Variables
local direction = 1.0
local ground    = false
local speed     = {0.0, 0.0, 0.0}

-- Components
local animator      = nil
local mastersensor  = nil
local bottomsensor  = nil
local bottomLsensor = nil
local bottomRsensor = nil
local ledgeLsensor  = nil
local ledgeRsensor  = nil
local leftsensor    = nil
local rightsensor   = nil
local topsensor     = nil


local clamp = function(x, min, max)
   return math.min(math.max(x, min), max)
end

function init()
   entity.setProperty(1, true)
   entity.setProperty(2, false)
   entity.setProperty(3, false)
   entity.translate({128, 128, 0}, true)
   animator = entity.getComponent("animator")
   
   -- Get sensors
   mastersensor  = entity.getComponent("MasterSensor")
   bottomsensor  = entity.getComponent("BottomSensor")
   bottomLsensor = entity.getComponent("BottomLSensor")
   bottomRsensor = entity.getComponent("BottomRSensor")
   ledgeLsensor  = entity.getComponent("LedgeLSensor")
   ledgeRsensor  = entity.getComponent("LedgeRSensor")
   leftsensor    = entity.getComponent("LeftSensor")
   rightsensor   = entity.getComponent("RightSensor")
   topsensor     = entity.getComponent("TopSensor")
end

function update(dt)
   local pos = entity.getPosition()
   local lstick = common.lstick()

   -- Y axis movement
   if not ground then speed[Y] = speed[Y] + gravity end
   if ground and common.btntap(PAD_A) then
      ground = false
      speed[Y] = jmpStg
   end

   -- X axis movement
   speed[X] = speed[X] + (lstick[X] * accel)
   speed[X] = clamp(speed[X], -maxSpd, maxSpd)
   if ground then
      if lstick[X] == 0.0 then
         if speed[X] > 0.0 then speed[X] = speed[X] - decel
         elseif speed[X] < 0.0 then speed[X] = speed[X] + decel
         end
         
         if math.abs(speed[X]) < decel then speed[X] = 0.0 end
      elseif lstick[X] < 0.0 and speed[X] > 0.0 then
         speed[X] = speed[X] - decel * 2.0
      elseif lstick[X] > 0.0 and speed[X] < 0.0 then
         speed[X] = speed[X] + decel * 2.0
      end
   end
   if common.btnpress(PAD_X) then maxSpd = runMaxSpd
   else maxSpd = defMaxSpd end

   -- Collision Detection
   ground = false
   for key, obj in pairs(entity.getNearest()) do
      if not entity.getProperty(1, obj) then
         local objBV = entity.getComponent("AABB", obj)
         local solidpos = entity.getPosition(obj)
         local solidsz  = {entity.getMagnification(X, obj),
                           entity.getMagnification(Y, obj),
                           entity.getMagnification(Z, obj)}
         
         -- Ground collision
         if (not ground)                     -- No ground found previously
            and (speed[Y] >= 0.0)            -- Player is not going up
            and (entity.getProperty(2, obj)) -- Is Solid
            and (entity.bv.isOverlapping(bottomsensor, objBV)
                    or entity.bv.isOverlapping(bottomLsensor, objBV)
                    or entity.bv.isOverlapping(bottomRsensor, objBV))
         then
            -- It is indeed a solid object.
            -- Ground collisions ahoy
            pos[Y] = solidpos[Y] - hitboxRad[Y]
            speed[Y] = 0.0
            ground = true
         end

         -- Top collision
         if speed[Y] < 0.0                     -- Player is going up
            and entity.getProperty(2, obj)     -- Is Solid
            and not entity.getProperty(3, obj) -- Is NOT jumpthru
            and entity.bv.isOverlapping(topsensor, objBV)
         then
            pos[Y] = (solidpos[Y] + solidsz[Y]) + hitboxRad[Y]
            speed[Y] = 0.0
         end

         -- Left collision
         if (speed[X] < 0.0)                     -- Player is walking
            and entity.getProperty(2, obj)       -- Is Solid
            and (not entity.getProperty(3, obj)) -- Is NOT jumpthru
            and entity.bv.isOverlapping(leftsensor, objBV)
         then
            pos[X] = (solidpos[X] + solidsz[X]) + hitboxRad[X]
            speed[X] = 0.0
         end

         -- Right collision
         if (speed[X] > 0.0)                      -- Player is walking
            and entity.getProperty(2, obj)        -- Is Solid
            and (not entity.getProperty(3, obj))  -- Is NOT jumpthru
            and entity.bv.isOverlapping(rightsensor, objBV)
         then
            pos[X] = solidpos[X] - hitboxRad[X]
            speed[X] = 0.0
         end
      end
   end

   
   -- Platformer-like jump
   if not ground
      and speed[Y] < minJmp
      and not common.btnpress(PAD_A)
   then speed[Y] = minJmp end

   -- Transform position
   pos[X] = pos[X] + speed[X]
   pos[Y] = pos[Y] + speed[Y]
   pos[Z] = pos[Z] + speed[Z]

   -- Hand position back to engine
   entity.translate(pos, true)

   -- Direction
   if speed[X] > 0.0 then
      direction = 1.0
   elseif speed[X] < 0.0 then
      direction = -1.0
   end
   entity.scale({direction, 1, 1}, true)
   
   -- Animation
   if ground then
      if speed[X] == 0.0 then -- If not moving, stop
         render.animator.setAnimation(animator, "stopped")
      else
         render.animator.setAnimation(animator, "walking")
         local norm = 1.0 - (math.abs(speed[X]) / runMaxSpd)
         local animspd = norm * 6.0
         render.animator.setSpeed(animator,
                                  animspd + render.animator.getDefaultSpeed(animator))
      end
   else
      render.animator.setAnimation(animator, "jumping")
   end
end
