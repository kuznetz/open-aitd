#pragma once
#include "../resources/resources.h"
#include "../world/world.h"
#include <iostream>

namespace openAITD {

  using namespace std;
	using namespace raylib;

	class TracksController {
	public:
		World* world;

		TracksController(World* world) {
			this->world = world;
		}

		void rotateTo(GameObject& gobj, const Vector3& target, const float timeDelta, const float rotateSpeed = 0.5f * PI)
		{
			Matrix rotMatrix = gobj.getRotMatrix();
			//Z+ forward
			Vector3 forward = { rotMatrix.m8, rotMatrix.m9, rotMatrix.m10 };

			const auto& pos = gobj.getPosition();
			Vector2 targetDir = { target.x - pos.x, target.z - pos.z };
			float lenSq = targetDir.x * targetDir.x + targetDir.y * targetDir.y;
			if (lenSq < 1e-10f) return;
			targetDir = Vector2Normalize(targetDir);

    float targetYaw = atan2(targetDir.x, targetDir.y);
    float currentYaw = atan2(forward.x, forward.z);
    float diff = EulerAngles::NormalizeAngle(targetYaw - currentYaw);

			EulerAngles euler = gobj.getOrigRotation();
			const float eps = 0.001f;
			if (fabs(diff) < eps) {
				  printf("forw\n");
					euler.y = targetYaw;
					gobj.setOrigRotation(euler);
					return;
			}
			float maxStep = rotateSpeed * timeDelta;
			if (fabs(diff) <= maxStep) {
				  printf("forw+\n");
					euler.y = targetYaw;
			} else {
				  printf((string("rot ") + (diff > 0?"+":"-") + "\n").c_str());
					euler.y += (diff > 0 ? maxStep : -maxStep);
			}
			euler.y = EulerAngles::NormalizeAngle(euler.y);
			gobj.setOrigRotation(euler);
		}


		bool gotoPos(GameObject& gobj, TrackItem& trackItm, const float timeDelta) {
			Vector3 targetPos = trackItm.pos;
			targetPos = { trackItm.pos.x, gobj.getPosition().y, trackItm.pos.z };
			if (trackItm.room != gobj.getRoomId())
			{
				const auto& objRoomPos = world->curStage->rooms[gobj.getRoomId()].origPosition;
				const auto& trackItmRoomPos = world->curStage->rooms[trackItm.room].origPosition;
				targetPos.x += trackItmRoomPos.x - objRoomPos.x;
				targetPos.z += trackItmRoomPos.z - objRoomPos.z;
			}
			gobj.track.targetPos = targetPos;			
			rotateTo(gobj, gobj.track.targetPos, timeDelta);

			//gobj.track.direction = Vector3Normalize(Vector3Subtract(gobj.track.target, gobj.getPosition()));
		    //float nextDistanceToPoint = Vector3DistanceSqr(Vector3Add(gobj.getPosition(), gobj.track.direction), gobj.track.target);
			//DISTANCE_TO_POINT_TRESSHOLD = 0.1m
			//TODO: change code 4 distance reach

			float distanceToPoint = Vector3DistanceSqr(gobj.getPosition(), gobj.track.targetPos);
			if (distanceToPoint >= 0.1 || gobj.rotateAnim.timeEnd > 0) // || distanceToPoint >= nextDistanceToPoint
			{
				// not yet at position
				gobj.track.posStarted = true;
				return false;
			}
			else // reached position
			{
				//gobj.getPosition() = gobj.track.target;
				return true;
			}
		}

		bool gotoPos3D(GameObject& gobj, TrackItem& trackItm) {
			Vector3 targetPos = trackItm.pos;
			gobj.track.startPos = gobj.getPosition();
			gobj.track.targetPos = targetPos;
			//trackItm.time -= deltaTime;
			//rotateTo(gobj, gobj.track.target);
			
			raylib::Vector2 objPos2D = { gobj.getPosition().x, gobj.getPosition().z };
			raylib::Vector2 targetPos2D = { targetPos.x, targetPos.z };
			if ( Vector2DistanceSqr(objPos2D, targetPos2D) > (0.1*0.1) )
			//if (Vector3DistanceSqr(gobj.getPosition(), gobj.track.targetPos) > (0.05*0.05) )
			{				
				gobj.track.posStarted = true;
				return false;
			}
			else // reached position
			{
				gobj.getPosition() = gobj.track.targetPos;
				return true;
			}
		}

		bool gotoStairs(GameObject& gobj, TrackItem& trackItm, bool zCoord, const float timeDelta) {
				Vector3 pos = gobj.getPosition();
				if (!gobj.track.posStarted) {
						gobj.track.startPos = pos;
						float distY = trackItm.pos.y - pos.y;
						float distX = (zCoord) ? 
								abs(trackItm.pos.z - pos.z) :
								abs(trackItm.pos.x - pos.x);
						gobj.track.direction.y = distY / distX;
				}

				Vector3 target = trackItm.pos;
				target.y = pos.y;
				rotateTo(gobj, target, timeDelta);

				float diff = (zCoord) ?
						(gobj.track.startPos.z - pos.z) :
						(gobj.track.startPos.x - pos.x);
				
				pos.y = gobj.track.startPos.y + (gobj.track.direction.y * abs(diff));
				gobj.setPosition(pos);

				if ( 
						(gobj.track.direction.y > 0 && pos.y < trackItm.pos.y) ||
						(gobj.track.direction.y < 0 && pos.y > trackItm.pos.y)
				) {
						gobj.track.posStarted = true;
						return false;
				}
				else
				{
						gobj.setPosition(trackItm.pos);
						return true;
				}
		}

		void rotateXYZ(GameObject& gobj, TrackItem& trackItm) {
			auto& r = gobj.getOrigRotation();
      r.x = trackItm.rot.x;
			r.y = trackItm.rot.y;
			r.z = trackItm.rot.z;
		}

		void processObjTrack( GameObject& gobj, const float timeDelta ) {
			if (gobj.track.id == -1) return;
			if (gobj.id == 1) {
				gobj.animation.id = world->player.animations.walkForw;
			}

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
			  gobj.changeRoom(trackItm.room);
				gobj.setPosition(trackItm.pos);
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
			if (gobj.getStageId() != gobj2.getStageId()) return;
			auto pos2 = world->curStage->VectorChangeRoom(gobj2.getPosition(), gobj2.getRoomId(), gobj.getRoomId());
			
			Vector3 v2 = Vector3Subtract(pos2, gobj.getPosition());
			//printf("Rooms %d %d\n", gobj.getRoomId(), gobj2.getRoomId());
			//printf("VectorChangeRoom %f %f\n", v2.x, v2.z);

			rotateTo(gobj, pos2, timeDelta);
		}


	};

}
