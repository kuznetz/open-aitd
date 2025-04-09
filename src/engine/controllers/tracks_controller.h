#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;
namespace openAITD {

	class TracksController {
	public:
		World* world;

		TracksController(World* world) {
			this->world = world;
		}

		bool gotoPos(GameObject& gobj, TrackItem& trackItm, float timeDelta) {
			
			Vector3 targetPos = trackItm.pos;
			targetPos = { trackItm.pos.x, 0, -trackItm.pos.z };
			if (trackItm.room != gobj.location.roomId)
			{
				targetPos.x -= world->curStage->rooms[gobj.location.roomId].position.x - world->curStage->rooms[trackItm.room].position.x;
				targetPos.z += world->curStage->rooms[gobj.location.roomId].position.z - world->curStage->rooms[trackItm.room].position.z;
			}

			gobj.track.target = targetPos;

			float distanceToPoint = Vector3DistanceSqr(gobj.location.position, targetPos);
			//DISTANCE_TO_POINT_TRESSHOLD = 0.1m
			if (distanceToPoint >= 0.0001) // not yet at position
			{
				///Vector3 forw = Vector3RotateByQuaternion({ 0, 0, 1 }, gobj.location.rotation);
				//TODO: CACHE targetV
				Vector3 targetV = Vector3Normalize(Vector3Subtract(targetPos, gobj.location.position));
				//gobj.location.rotation = QuaternionFromVector3ToVector3({ 0,0,1 }, targetV);

				//TODO: rotate

                //speed = 4;
				gobj.physics.moveVec = Vector3Scale(targetV, 1.2*timeDelta);
				return false;
			}
			else // reached position
			{
				gobj.location.position = targetPos;
				return true;
			}
		}

		void rotateXYZ(GameObject& gobj, TrackItem& trackItm) {
			Matrix mx = MatrixRotateX(trackItm.rot.x);
			Matrix my = MatrixRotateY(trackItm.rot.y); // + PI
			Matrix mz = MatrixRotateZ(trackItm.rot.z);
			Matrix matRotation = MatrixMultiply(MatrixMultiply(my, mx), mz);
			matRotation = MatrixTranspose(matRotation);
			gobj.location.rotation = QuaternionFromMatrix(matRotation);
		}

		void process(float timeDelta) {
			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj = world->gobjects[i];
				if (!gobj.moveFlag) continue;
				if (gobj.track.mode != GOTrackMode::track || gobj.track.id == -1) continue;
				if (i == 288) {
					printf("288");
				}

				while (true) {
					auto& trackItm = world->resources->tracks[gobj.track.id][gobj.track.pos];
					bool nextPos = false;
					switch (trackItm.type) {
					case TrackItemType::GOTO_POS:
						nextPos = gotoPos(gobj, trackItm, timeDelta);
						break;
					case TrackItemType::MARK:
						gobj.track.mark = trackItm.mark;
						nextPos = true;
						break;
					case TrackItemType::REWIND:
						gobj.track.pos = -1;
						nextPos = true;
						break;
					case TrackItemType::END:
						gobj.track.mode = GOTrackMode::none;
						gobj.track.id = -1;
						gobj.track.pos = 0;
						gobj.track.mark = 0;
						break;
					case TrackItemType::ROTATE_XYZ:
						rotateXYZ(gobj, trackItm);
						nextPos = true;
						break;
					default:
						cout << "unkn TrackItemType " << to_string((int)trackItm.type) << endl;
						nextPos = true;
					}
					if (nextPos) gobj.track.pos++;// else break;
					break;
				}

				gobj.moveFlag = false;
			}
		}
	};

}
