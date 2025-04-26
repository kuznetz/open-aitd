#pragma once
#include "../resources/resources.h"
#include "../world/world.h"
#include <iostream>

using namespace std;
namespace openAITD {

	class TracksController {
	public:
		World* world;

		TracksController(World* world) {
			this->world = world;
		}

		void rotateTo(GameObject& gobj, const Vector3& target) {
			if (!gobj.track.posStarted) {
				gobj.rotateAnim.curTime = 0;
				gobj.rotateAnim.timeEnd = 0.25;
				gobj.rotateAnim.from = gobj.location.rotation;
			}
			auto& m = MatrixTranspose(MatrixLookAt(gobj.location.position, target, { 0,1,0 }));
			auto rotTo = QuaternionFromMatrix(m);
			if (gobj.rotateAnim.timeEnd > 0) {
				gobj.rotateAnim.to = rotTo;
				//gobj.track.direction = Vector3Normalize(Vector3Subtract(gobj.track.target, gobj.location.position));
			}
			else {
				gobj.location.rotation = rotTo;
			}
		}

		bool gotoPos(GameObject& gobj, TrackItem& trackItm) {
			Vector3 targetPos = trackItm.pos;
			targetPos = { trackItm.pos.x, gobj.location.position.y, trackItm.pos.z };
			if (trackItm.room != gobj.location.roomId)
			{
				targetPos.x += world->curStage->rooms[trackItm.room].position.x - world->curStage->rooms[gobj.location.roomId].position.x;
				targetPos.z += world->curStage->rooms[trackItm.room].position.z - world->curStage->rooms[gobj.location.roomId].position.z;
			}
			//targetPos.z = -targetPos.z;
			gobj.track.target = targetPos;			
			rotateTo(gobj, gobj.track.target);

			//gobj.track.direction = Vector3Normalize(Vector3Subtract(gobj.track.target, gobj.location.position));
		    //float nextDistanceToPoint = Vector3DistanceSqr(Vector3Add(gobj.location.position, gobj.track.direction), gobj.track.target);
			//DISTANCE_TO_POINT_TRESSHOLD = 0.1m
			//TODO: change code 4 distance reach

			float distanceToPoint = Vector3DistanceSqr(gobj.location.position, gobj.track.target);
			if (distanceToPoint >= 0.1 || gobj.rotateAnim.timeEnd > 0) // || distanceToPoint >= nextDistanceToPoint
			{
				// not yet at position
				gobj.track.posStarted = true;
				return false;
			}
			else // reached position
			{
				//gobj.location.position = gobj.track.target;
				return true;
			}
		}

		bool gotoStairs(GameObject& gobj, TrackItem& trackItm, bool zCoord) {
			if (!gobj.track.posStarted) {
				gobj.track.start = gobj.location.position;
				float distY = trackItm.pos.y - gobj.location.position.y;
				float distX = (zCoord)? 
					abs(trackItm.pos.z - gobj.location.position.z):
					abs(trackItm.pos.x - gobj.location.position.x);
				gobj.track.direction.y = distY / distX;
			}

			Vector3 target = trackItm.pos;
			target.y = gobj.location.position.y;
			rotateTo(gobj, target);

			float diff = (zCoord) ?
				(gobj.track.start.z - gobj.location.position.z):
				(gobj.track.start.x - gobj.location.position.x);
			gobj.location.position.y = gobj.track.start.y + (gobj.track.direction.y * abs(diff));

			if ( 
				(gobj.track.direction.y > 0 && gobj.location.position.y < trackItm.pos.y) ||
				(gobj.track.direction.y < 0 && gobj.location.position.y > trackItm.pos.y)
			) {
				// not yet at position
				gobj.track.posStarted = true;
				return false;
			}
			else // reached position
			{
				gobj.location.position = trackItm.pos;
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

		void processObjTrack( GameObject& gobj ) {
			if (gobj.track.id == -1) return;
			auto& trackItm = world->resources->tracks[gobj.track.id][gobj.track.pos];
			bool nextPos = true;
			switch (trackItm.type) {
			case TrackItemType::GOTO_POS:
				//cout << "GOTO_POS" << endl;
				nextPos = gotoPos(gobj, trackItm);
				break;
			case TrackItemType::MARK:
				gobj.track.mark = trackItm.mark;
				break;
			case TrackItemType::REWIND:
				gobj.track.pos = -1;
				break;
			case TrackItemType::END:
				gobj.track.mode = GOTrackMode::none;
				gobj.track.id = -1;
				gobj.track.pos = 0;
				gobj.track.mark = 0;
				nextPos = false;
				//cout << "track END" << endl;
				break;
			case TrackItemType::ROTATE_XYZ:
				//cout << "ROTATE_XYZ" << endl;
				rotateXYZ(gobj, trackItm);
				break;

			case TrackItemType::STAIRS_X:
				//trackItm.pos						
				nextPos = gotoStairs(gobj, trackItm, false);
				break;

			case TrackItemType::STAIRS_Z:
				//cout << "STAIRS_Z " << to_string(gobj.physics.collidable) << endl;
				nextPos = gotoStairs(gobj, trackItm, true);
				break;

			case TrackItemType::COLLISION_DISABLE:
				//cout << "COLLISION_DISABLE" << endl;
				gobj.physics.collidable = false;
				break;
			case TrackItemType::COLLISION_ENABLE:
				//cout << "COLLISION_ENABLE" << endl;
				gobj.physics.collidable = true;
				break;
			case TrackItemType::TRIGGERS_DISABLE:
				//cout << "TRIGGERS_DISABLE" << endl;
				gobj.bitField.trigger = 0;
				break;
			case TrackItemType::TRIGGERS_ENABLE:
				//cout << "TRIGGERS_ENABLE" << endl;
				gobj.bitField.trigger = 1;
				break;
			case TrackItemType::WARP:
				gobj.location.roomId = trackItm.room;
				gobj.location.position = trackItm.pos;
				break;
			default:
				cout << "unkn TrackItemType " << to_string((int)trackItm.type) << endl;
			}
			if (nextPos) {
				gobj.track.posStarted = false;
				gobj.track.pos++;// else break;
			}
		}

		void processObjFollow(GameObject& gobj) {
			if (gobj.track.id == -1) return;
			auto& gobj2 = world->gobjects[gobj.track.id];
			if (gobj.location.stageId != gobj2.location.stageId) return;
			auto pos2 = world->VectorChangeRoom(gobj2.location.position, gobj2.location.roomId, gobj.location.roomId);
			//Vector3 v1 = Vector3RotateByQuaternion({0,0,1}, gobj.location.rotation);
			Vector3 v2 = Vector3Subtract(pos2, gobj.location.position);
			//auto qDiff = QuaternionFromVector3ToVector3(v1, v2);
			//gobj.location.rotation = QuaternionMultiply(gobj.location.rotation, qDiff);
			//auto qDiff = QuaternionFromVector3ToVector3({0,0,1}, v2);
			//gobj.location.rotation = qDiff;

			rotateTo(gobj, pos2);
		}


	};

}
