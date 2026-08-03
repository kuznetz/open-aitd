#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

namespace openAITD {

class CameraController {
public:
    World& world;
    Resources& resources;

    CameraController(World& world)
        : world(world), resources(*world.resources) {}

    // Main camera update method (follows the target)
    void process() {
        if (!world.followTarget) return;
        auto& folObj = *world.followTarget;
        if (folObj.getStageId() == -1) return;

        // Switch the current scene to the one containing the target
        world.setCurStage(folObj.getStageId(), folObj.getRoomId());

        // If the target is still in a different scene (unlikely), exit early
        if (world.curStageId != folObj.getStageId()) return;

        // Compute the camera position relative to the room
        Vector3 pos = Vector3Add(folObj.getPosition(), world.curRoom->origPosition);

        // Find the closest camera in the current scene
        auto camId = world.curStage->closestCamera(pos);
        if (camId != -1) {
            world.curCameraId = camId;
        }
    }
};

}