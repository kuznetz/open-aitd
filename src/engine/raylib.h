#pragma once
namespace raylib { 
	#include <raylib.h>
	#include <raymath.h>
	#include <rcamera.h>
	#include <rlgl.h>
	#include <external/stb_image_resize2.h>

	inline BoundingBox correctBounds(const BoundingBox& b) {
		BoundingBox r = b;
		if (b.min.x > b.max.x) {
			r.min.x = b.max.x;
			r.max.x = b.min.x;
		}
		if (b.min.y > b.max.y) {
			r.min.y = b.max.y;
			r.max.y = b.min.y;
		}
		if (b.min.z > b.max.z) {
			r.min.z = b.max.z;
			r.max.z = b.min.z;
		}
		return  r;
	}

}