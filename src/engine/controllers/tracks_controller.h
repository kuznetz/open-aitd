﻿#pragma once
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
			targetPos = { trackItm.pos.x, 0, trackItm.pos.z };
			if (trackItm.room != gobj.location.roomId)
			{
				targetPos.x += world->curStage->rooms[trackItm.room].position.x - world->curStage->rooms[gobj.location.roomId].position.x;
				targetPos.z += world->curStage->rooms[trackItm.room].position.z - world->curStage->rooms[gobj.location.roomId].position.z;
			}
			//targetPos.z = -targetPos.z;
			gobj.track.target = targetPos;

			if (!gobj.track.posStarted) {
				gobj.rotateAnim.curTime = 0;
				gobj.rotateAnim.timeEnd = 0.25;
				gobj.rotateAnim.from = gobj.location.rotation;
			}			

			if (gobj.rotateAnim.timeEnd > 0) {
				auto& m = MatrixTranspose(MatrixLookAt(gobj.location.position, gobj.track.target, { 0,1,0 }));
				auto rotTo = QuaternionFromMatrix(m);
				gobj.rotateAnim.to = rotTo;
				gobj.track.direction = Vector3Normalize(Vector3Subtract(gobj.track.target, gobj.location.position));
			}

			float distanceToPoint = Vector3DistanceSqr(gobj.location.position, gobj.track.target);
			float nextDistanceToPoint = Vector3DistanceSqr(Vector3Add(gobj.location.position, gobj.track.direction), gobj.track.target);
			//DISTANCE_TO_POINT_TRESSHOLD = 0.1m
			//TODO: change code 4 distance reach
			if (distanceToPoint >= 0.01) // || distanceToPoint >= nextDistanceToPoint
			{
				// not yet at position
				gobj.track.posStarted = true;
				return false;
			}
			else // reached position
			{
				gobj.location.position = gobj.track.target;
				return true;
			}
		}

		void rotateXYZ(GameObject& gobj, TrackItem& trackItm) {
			Matrix mx = MatrixRotateX(trackItm.rot.x);
			Matrix my = MatrixRotateY(trackItm.rot.y); // + PI
			Matrix mz = MatrixRotateZ(trackItm.rot.z);
			Matrix matRotation = MatrixMultiply(MatrixMultiply(my, mx), mz);
			matRotation = MatrixTranspose(matRotation);
			gobj.location.rotation = QuaternionInvert(QuaternionFromMatrix(matRotation));
		}

		void process(float timeDelta) {
			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj = world->gobjects[i];
				if (!gobj.moveFlag) continue;
				if (gobj.track.mode != GOTrackMode::track || gobj.track.id == -1) continue;

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
					if (nextPos) {
						gobj.track.posStarted = false;
						gobj.track.pos++;// else break;
					}
					break;
				}

				gobj.moveFlag = false;
			}
		}
	};

}
