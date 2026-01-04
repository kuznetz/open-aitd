#pragma once
#include <algorithm>
#include "../raylib-cpp.h"

//using namespace std;
using namespace raylib;
namespace openAITD {

	class Bounds {
	public:
		static Vector3 CalculateMTV_XYZ(const Bounds& b1, const Bounds& b2)
		{
				float overlapX = std::min(b1.max.x, b2.max.x) - std::max(b1.min.x, b2.min.x);
				float overlapY = std::min(b1.max.y, b2.max.y) - std::max(b1.min.y, b2.min.y);
				float overlapZ = std::min(b1.max.z, b2.max.z) - std::max(b1.min.z, b2.min.z);

				if (overlapX <= 0 || overlapY <= 0 || overlapZ <= 0) {
						return { 0, 0, 0 };
				}

				float minOverlap = std::min({ abs(overlapX), abs(overlapY), abs(overlapZ) });

				Vector3 mtv = { 0, 0, 0 };
				if (minOverlap == abs(overlapX)) {
						mtv.x = overlapX * ((b1.min.x + b1.max.x) / 2 >= (b2.min.x + b2.max.x) / 2 ? -1 : 1);
				}
				else if (minOverlap == abs(overlapY)) {
						mtv.y = overlapY * ((b1.min.y + b1.max.y) / 2 >= (b2.min.y + b2.max.y) / 2 ? -1 : 1);
				}
				else {
						mtv.z = overlapZ * ((b1.min.z + b1.max.z) / 2 >= (b2.min.z + b2.max.z) / 2 ? -1 : 1);
				}

				return mtv;
		}

		static Vector3 CalculateMTV_XZ(const Bounds& b1, const Bounds& b2)
		{
			float overlapX = std::min(b1.max.x, b2.max.x) - std::max(b1.min.x, b2.min.x);
			float overlapZ = std::min(b1.max.z, b2.max.z) - std::max(b1.min.z, b2.min.z);
			if (overlapX <= 0 || overlapZ <= 0) {
				return { 0,0,0 };
			}
			bool pushAlongX = abs(overlapX) < abs(overlapZ);
			Vector3 mtv = { 0,0,0 };
			if (pushAlongX) {
				mtv.x = overlapX * ((b1.min.x + b1.max.x) / 2 >= (b2.min.x + b2.max.x) / 2 ? -1 : 1);
			}
			else {
				mtv.z = overlapZ * ((b1.min.z + b1.max.z) / 2 >= (b2.min.z + b2.max.z) / 2 ? -1 : 1);
			}
			return mtv;
		}
		
		Vector3 min = { 0,0,0 }; // Minimum vertex box-corner
		Vector3 max = { 0,0,0 }; // Maximum vertex box-corner

		Bounds(const Bounds& b) {
			this->min = b.min;
			this->max = b.max;
		}
		Bounds(Vector3 min, Vector3 max) {
			this->min = min;
			this->max = max;
		}
		Bounds() {
		}

		void correctBounds() {
			Bounds b(*this);
			if (b.min.x > b.max.x) {
				this->min.x = b.max.x;
				this->max.x = b.min.x;
			}
			if (b.min.y > b.max.y) {
				this->min.y = b.max.y;
				this->max.y = b.min.y;
			}
			if (b.min.z > b.max.z) {
				this->min.z = b.max.z;
				this->max.z = b.min.z;
			}
		}

		Bounds getExpanded(float x) {
			Bounds r(*this);
			r.min.x -= x;
			r.max.x += x;
			r.min.y -= x;
			r.max.y += x;
			r.min.z -= x;
			r.max.z += x;
			return r;
		}

		Bounds getCubeBounds()
		{
			Bounds r(*this);
			r.max.z = r.max.x = (max.x + max.z) / 2;
			r.min.z = r.min.x = -r.max.z;
			return r;
		}

		Bounds getRotatedBounds(const Quaternion& q)
		{
			auto& b = *this;
			Vector3 v[8];
			v[0] = { b.min.x, b.min.y, b.min.z };
			v[1] = { b.max.x, b.min.y, b.min.z };
			v[2] = { b.min.x, b.min.y, b.max.z };
			v[3] = { b.max.x, b.min.y, b.max.z };
			v[4] = { b.min.x, b.max.y, b.min.z };
			v[5] = { b.max.x, b.max.y, b.min.z };
			v[6] = { b.min.x, b.max.y, b.max.z };
			v[7] = { b.max.x, b.max.y, b.max.z };
			Bounds res;
			for (int i = 0; i < 8; i++) {
				v[i] = Vector3RotateByQuaternion(v[i], q);
				if (i == 0 || v[i].x < res.min.x) {
					res.min.x = v[i].x;
				}
				if (i == 0 || v[i].x > res.max.x) {
					res.max.x = v[i].x;
				}
				if (i == 0 || v[i].y < res.min.y) {
					res.min.y = v[i].y;
				}
				if (i == 0 || v[i].y > res.max.y) {
					res.max.y = v[i].y;
				}
				if (i == 0 || v[i].z < res.min.z) {
					res.min.z = v[i].z;
				}
				if (i == 0 || v[i].z > res.max.z) {
					res.max.z = v[i].z;
				}
			}
			return res;
		}

		bool CollToBoxV_XYZ(Vector3& v, Bounds& b2) {
			if (v.x == 0 && v.y == 0 && v.z == 0) return false;
			Bounds b1( Vector3Add(this->min, v), Vector3Add(this->max, v) );

			if (b1.max.x < b2.min.x || b1.min.x > b2.max.x)  return false;
			if (b1.max.y < b2.min.y || b1.min.y > b2.max.y)  return false;
			if (b1.max.z < b2.min.z || b1.min.z > b2.max.z)  return false;

			auto& mtv = CalculateMTV_XYZ(b1, b2);
			v.x -= mtv.x;
			v.x -= mtv.y;
			v.z -= mtv.z;
			return true;
		}

		bool CollToBoxV_XZ(Vector3& v, Bounds& b2) {
			if (v.x == 0 && v.z == 0) return false;
			Bounds b1( Vector3Add(this->min, v), Vector3Add(this->max, v) );

			if (b1.max.x < b2.min.x || b1.min.x > b2.max.x)  return false;
			if (b1.max.z < b2.min.z || b1.min.z > b2.max.z)  return false;

			auto& mtv = CalculateMTV_XZ(b1, b2);
			v.x -= mtv.x;
			v.z -= mtv.z;
			return true;
		}

		bool CollToBox(Bounds& b2) {
			Bounds& b1 = *this;
			if (b1.max.x < b2.min.x || b1.min.x > b2.max.x)  return false;
			if (b1.max.y < b2.min.y || b1.min.y > b2.max.y)  return false;
			if (b1.max.z < b2.min.z || b1.min.z > b2.max.z)  return false;
			return true;
		}

	};

}