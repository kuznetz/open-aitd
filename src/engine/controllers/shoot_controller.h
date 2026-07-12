#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;

namespace openAITD {

    /**
     * ShootController handles rifle shooting via raycast.
     */
    class ShootController {
    public:
        World* world;
        Resources* resources;

        ShootController(World* world)
            : world(world)
            , resources(world->resources) {
        }

        /**
         * Performs a shot from the given origin in the given direction.
         * @param shooter   The GameObject that fires the shot (player or enemy).
         * @param origin    Start point of the ray (in shooter's room coordinates).
         * @param direction Direction of the ray (will be normalized internally).
         * @param range     Maximum distance of the shot.
         * @param damage    Damage dealt on hit.
         */
        void shoot(GameObject* shooter, const Vector3& origin, const Vector3& direction, int damage, float range = 1000) {
            if (!shooter) return;

            Vector3 dir = Vector3Normalize(direction);
            float minDist = range;
            GameObject* hitTarget = nullptr;
            bool hitStatic = false;   // true if the closest hit is a static collider

            // Check dynamic objects
            for (auto& gobj : world->gobjects) {
                if (gobj.id == shooter->id) continue;
                if (gobj.location.stageId != world->curStageId) continue;
                if (gobj.modelId == -1 || !gobj.physics.collidable) continue;

                Bounds bounds = world->getObjectBounds(gobj);
                // Transform bounds if the object is in a different room
                bounds = world->BoundsChangeRoom(bounds, gobj.location.roomId, shooter->location.roomId);

                float t;
                if (bounds.RayIntersect(origin, dir, t)) {
                    if (t > 0.0f && t < minDist) {
                        minDist = t;
                        hitTarget = &gobj;
                        hitStatic = false;
                    }
                }
            }

            // Check static colliders in the shooter's current room
            Room& room = resources->stages[world->curStageId].rooms[shooter->location.roomId];
            for (const auto& collider : room.colliders) {
                const Bounds& colB = collider.bounds;
                float t;
                if (colB.RayIntersect(origin, dir, t)) {
                    if (t > 0.0f && t < minDist) {
                        minDist = t;
                        hitTarget = nullptr;        // static collider is not a GameObject
                        hitStatic = true;
                    }
                }
            }

            world->debugShootFrom = origin;
            Vector3 hitPoint;
            if (hitTarget || hitStatic) {
                // Compute hit point along the ray
                hitPoint = Vector3Add(origin, Vector3Scale(dir, minDist));
                world->debugShootTo = hitPoint;

                if (hitTarget) {
                    // Apply damage and track hit
                    // hitTarget->damage.hitBy = shooter;
                    // hitTarget->damage.damage = damage;
                    // shooter->hit.hitTo = hitTarget;
                }
            } else {
                // No hit – ray ends at maximum range
                hitPoint = origin + dir * range;
                world->debugShootTo = hitPoint;
            }

        }
    };

}