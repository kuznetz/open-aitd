#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../raylib.h"

using namespace std;
namespace openAITD {

	struct RenderOrder {
		RenderOrder* next = 0;
		GameObject* gobj = 0;
		float zPos;
		Vector2 screenMin;
		Vector2 screenMax;
	};

}
