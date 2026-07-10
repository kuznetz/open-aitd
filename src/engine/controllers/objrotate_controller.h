#pragma once
#include "../resources/resources.h"
#include "../world/world.h"
#include <cmath>  // for fmod, PI (if not already included)

using namespace std;
namespace openAITD {

    // Interpolate two angles taking the shortest path
    inline float LerpAngle(float from, float to, float t) {
        // Compute difference and normalize it to [-PI, PI]
        float diff = to - from;
        diff = std::fmod(diff + PI, 2*PI); // bring to [0, 2*PI)
        if (diff < 0.0f) diff += 2*PI;
        diff -= PI; // now diff in [-PI, PI]

        // Linear interpolation along the shortest arc
        float result = from + diff * t;

        // Normalize result back to [-PI, PI] (to prevent error accumulation)
        result = std::fmod(result + PI, 2*PI);
        if (result < 0.0f) result += 2*PI;
        result -= PI;

        return result;
    }

    // Interpolate two Euler vectors component-wise using shortest path
    inline EulerAngles LerpEulerShortest(const EulerAngles& from, const EulerAngles& to, float t) {
        EulerAngles result;
        result.x = LerpAngle(from.x, to.x, t);
        result.y = LerpAngle(from.y, to.y, t);
        result.z = LerpAngle(from.z, to.z, t);        
        return result;
    }

    class ObjRotateController {
    public:
        World* world;
        Resources* resources;

        ObjRotateController(World* world) {
            this->world = world;
            this->resources = world->resources;
        }

        void process(float timeDelta) {
            Stage& curStage = resources->stages[world->curStageId];

            for (int i = 0; i < world->gobjects.size(); i++) {
                auto& gobj = world->gobjects[i];
                if (gobj.getStage() != world->curStageId) continue;
                if (gobj.rotateAnim.timeEnd <= 0) continue;

                auto& rot = gobj.rotateAnim;
                rot.curTime += timeDelta;

                if (rot.curTime >= rot.timeEnd) {
                    // Animation finished: set final rotation (already normalized)
                    gobj.location.rotation2 = rot.to;
                    rot.timeEnd = 0;
                    continue;
                }

                // Interpolate using the shortest angular path
                float t = rot.curTime / rot.timeEnd;
                gobj.location.rotation2 = LerpEulerShortest(rot.from, rot.to, t);

                // Invalidate cached bounds and check collisions for rotated objects
                if (gobj.boundsType == BoundsType::rotated) {
                    gobj.physics.boundsCached = false;
                    checkCollisions(gobj, curStage);
                }
            }
        }

    private:
        // Helper method to perform collision detection for a rotated object
        void checkCollisions(GameObject& gobj, const Stage& curStage) {
            // Get bounds of the current object once
            Bounds objB = world->getObjectBounds(gobj);

            for (int j = 0; j < world->gobjects.size(); j++) {
                auto& gobj2 = world->gobjects[j];
                if (&gobj == &gobj2) continue;
                if (gobj2.modelId == -1) continue;
                if (gobj2.location.stageId != gobj.location.stageId) continue;

                Bounds objB2 = world->getObjectBounds(gobj2);

                // If objects are in different rooms, check connectivity and adjust bounds
                if (gobj2.location.roomId != gobj.location.roomId) {
                    if (resources->isRoomsConnected(curStage, gobj.location.roomId, gobj2.location.roomId)) {
                        objB2 = world->BoundsChangeRoom(objB2, gobj2.location.roomId, gobj.location.roomId);
                    }
                    else {
                        continue;
                    }
                }

                bool c = objB.CollToBox(objB2);
                if (c) {
                    gobj.physics.objectColl = gobj2.id;
                    gobj2.physics.collidedBy = gobj.id;
                }
            }
        }
    };
}