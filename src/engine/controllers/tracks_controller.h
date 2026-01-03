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

		int calcRotateDirection(const Vector2& from, const Vector2& to) {
			float cross = from.x * to.y - from.y * to.x;
			float dot = from.x * to.x + from.y * to.y;
			float eps = 1e-9;			
			if (cross > eps) {
					return 2; //COUNTERCLOCKWISE;
			} else if (cross < -eps) {
					return 1; //CLOCKWISE;
			} else {
					if (dot > eps) {
							return 0; // COLLINEAR;
					} else if (dot < -eps) {
							return 3; // OPPOSITE
					} else {
							return 0; //Zero Vec
					}
			}
		}

		void rotateTo(GameObject& gobj, const Vector3& target, const float timeDelta) {
			//gobj.physics.moveVec
			raylib::Vector3 forw = { 0,0,-1 };
			forw = Vector3RotateByQuaternion(forw, gobj.location.rotation);
      raylib::Vector2 forward2D = Vector2Normalize({ forw.x, forw.z });
			gobj.track.debug.forward2D = forward2D;

			auto& pos = gobj.location.position;
			raylib::Vector2 targetDir = Vector2Normalize({
					target.x - pos.x,
					target.z - pos.z 
			});
			gobj.track.debug.targetDir = targetDir;
    
	    float angle = Vector2Angle(forward2D, targetDir);
  	  const float rotateSpeed = PI; // 180°/sec
      const float angleThreshold = 0.01f; // ~0.57°
			gobj.track.debug.angle = angle;
      
			if (fabs(angle) < angleThreshold) return;
    
	    //int dir = calcRotateDirection(forward2D, targetDir);
    	//if (dir == 0) return;
    	//float cw = (dir == 2) ? -1.0f : 1.0f;
			float cw = (angle > 0) ? -1.0f : 1.0f;
			auto q = QuaternionFromAxisAngle({ 0, 1, 0 }, rotateSpeed * timeDelta * cw);
      gobj.location.rotation = QuaternionMultiply(gobj.location.rotation, q);
		}

		bool gotoPos(GameObject& gobj, TrackItem& trackItm, const float timeDelta) {
			Vector3 targetPos = trackItm.pos;
			targetPos = { trackItm.pos.x, gobj.location.position.y, trackItm.pos.z };
			if (trackItm.room != gobj.location.roomId)
			{
				targetPos.x += world->curStage->rooms[trackItm.room].position.x - world->curStage->rooms[gobj.location.roomId].position.x;
				targetPos.z += world->curStage->rooms[trackItm.room].position.z - world->curStage->rooms[gobj.location.roomId].position.z;
			}
			//targetPos.z = -targetPos.z;
			gobj.track.targetPos = targetPos;			
			rotateTo(gobj, gobj.track.targetPos, timeDelta);

			//gobj.track.direction = Vector3Normalize(Vector3Subtract(gobj.track.target, gobj.location.position));
		    //float nextDistanceToPoint = Vector3DistanceSqr(Vector3Add(gobj.location.position, gobj.track.direction), gobj.track.target);
			//DISTANCE_TO_POINT_TRESSHOLD = 0.1m
			//TODO: change code 4 distance reach

			float distanceToPoint = Vector3DistanceSqr(gobj.location.position, gobj.track.targetPos);
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

		bool gotoPos3D(GameObject& gobj, TrackItem& trackItm) {
			Vector3 targetPos = trackItm.pos;
			gobj.track.startPos = gobj.location.position;
			gobj.track.targetPos = targetPos;
			//trackItm.time -= deltaTime;
			//rotateTo(gobj, gobj.track.target);
			
			raylib::Vector2 objPos2D = { gobj.location.position.x, gobj.location.position.z };
			raylib::Vector2 targetPos2D = { targetPos.x, targetPos.z };
			if ( Vector2DistanceSqr(objPos2D, targetPos2D) > (0.1*0.1) )
			//if (Vector3DistanceSqr(gobj.location.position, gobj.track.targetPos) > (0.05*0.05) )
			{				
				gobj.track.posStarted = true;
				return false;
			}
			else // reached position
			{
				gobj.location.position = gobj.track.targetPos;
				return true;
			}
		}

		bool gotoStairs(GameObject& gobj, TrackItem& trackItm, bool zCoord, const float timeDelta) {
			if (!gobj.track.posStarted) {
				gobj.track.startPos = gobj.location.position;
				float distY = trackItm.pos.y - gobj.location.position.y;
				float distX = (zCoord)? 
					abs(trackItm.pos.z - gobj.location.position.z):
					abs(trackItm.pos.x - gobj.location.position.x);
				gobj.track.direction.y = distY / distX;
			}

			Vector3 target = trackItm.pos;
			target.y = gobj.location.position.y;
			rotateTo(gobj, target, timeDelta);

			float diff = (zCoord) ?
				(gobj.track.startPos.z - gobj.location.position.z):
				(gobj.track.startPos.x - gobj.location.position.x);
			gobj.location.position.y = gobj.track.startPos.y + (gobj.track.direction.y * abs(diff));

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

		void processObjTrack( GameObject& gobj, const float timeDelta ) {
			if (gobj.track.id == -1) return;
			auto& trackItm = world->resources->tracks[gobj.track.id][gobj.track.pos];
			bool nextPos = true;
			switch (trackItm.type) {
			case TrackItemType::GOTO_POS:
				//cout << "GOTO_POS" << endl;
				nextPos = gotoPos(gobj, trackItm, timeDelta);
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
			case TrackItemType::GOTO_3D:
				nextPos = gotoPos3D(gobj, trackItm);
				break;
			case TrackItemType::STAIRS_X:
				//trackItm.pos						
				nextPos = gotoStairs(gobj, trackItm, false, timeDelta);
				break;

			case TrackItemType::STAIRS_Z:
				//cout << "STAIRS_Z " << to_string(gobj.physics.collidable) << endl;
				nextPos = gotoStairs(gobj, trackItm, true, timeDelta);
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

		void processObjFollow(GameObject& gobj, const float timeDelta) {
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

			rotateTo(gobj, pos2, timeDelta);
		}


	};

}
