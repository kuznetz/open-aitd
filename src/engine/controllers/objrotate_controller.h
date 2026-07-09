#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;
namespace openAITD {

    class ObjRotateController {
    public:
        World* world;
        Resources* resources;

        ObjRotateController(World* world) {
            this->world = world;
            this->resources = world->resources;
        }

        void process(float timeDelta) {
            auto& curStage = resources->stages[world->curStageId];

            for (int i = 0; i < world->gobjects.size(); i++) {
                auto& gobj = world->gobjects[i];
                if (gobj.location.stageId != world->curStageId) continue;
                if (gobj.rotateAnim.timeEnd <= 0) continue;

                // Save old rotation in case we need to revert
                auto oldRotate = gobj.location.rotation2;   // now Vector3 Euler rad

                auto& rot = gobj.rotateAnim;

                // Start of animation: set rotation to 'from' Euler vector
                if (rot.curTime == 0.0f) {
                    gobj.location.rotation2 = rot.from;     // rot.from is now Vector3 Euler rad
                }

                rot.curTime += timeDelta;

                if (rot.curTime >= rot.timeEnd) {
                    // Animation ends
                    gobj.location.rotation2 = rot.to;       // rot.to is Vector3 Euler rad
                    gobj.location.rotOrig = rot.toOrig;    // assume rotOrig is also Vector3
                    rot.timeEnd = 0;
                    continue;
                }
                else {
                    // Interpolate between from and to using Euler spherical interpolation
                    float t = rot.curTime / rot.timeEnd;
                    gobj.location.rotation2 = Vector3Lerp(rot.from, rot.to, t);
                }

                if (gobj.boundsType == BoundsType::rotated) {
                    gobj.physics.boundsCached = false;
                }

                // Collision check for rotated objects
                if (gobj.boundsType == BoundsType::rotated) {
                    auto* curRoom = &curStage.rooms[gobj.location.roomId];

                    Bounds& objB = world->getObjectBounds(gobj);
                    bool collided = false;
                    for (int j = 0; j < world->gobjects.size(); j++) {
                        auto& gobj2 = world->gobjects[j];
                        if (&gobj == &gobj2) continue;
                        if (gobj2.modelId == -1) continue;
                        if (gobj2.location.stageId != gobj.location.stageId) continue;

                        Bounds objB2 = world->getObjectBounds(gobj2);
                        if (gobj2.location.roomId != gobj.location.roomId) {
                            if (resources->isRoomsConnected(*world->curStage, gobj.location.roomId, gobj2.location.roomId)) {
                                objB2 = world->BoundsChangeRoom(objB2, gobj2.location.roomId, gobj.location.roomId);
                            }
                            else {
                                continue;
                            }
                        }
                        bool c = objB.CollToBox(objB2);
                        collided = collided || c;
                        if (c) {
                            gobj.physics.objectColl = gobj2.id;
                            gobj2.physics.collidedBy = gobj.id;
                        }
                    }

                    // Optional: prevent rotation on collision (commented out as before)
                    // if (collided) {
                    //     gobj.rotateAnim.curTime = oldTime;
                    //     gobj.rotateAnim.timeEnd = 0;
                    //     gobj.location.rotation = oldRotate;
                    // }
                }
            }
        }
    };

}