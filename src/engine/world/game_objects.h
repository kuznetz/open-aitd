#pragma once
#include <vector>
#include <string>
#include <raymath.h>

using namespace std;
namespace openAITD {

	class GameObject
	{
	public:
		int ownerIdx;
		int modelIdx;
		int animationIdx;
		int lifeIdx;
		int lifeMode;
		int traceIdx;
		int stageIdx;
		int roomIdx;
		Vector3 position;
		Vector3 rotation;
		Vector4 rotationQ;
		int animationFrame;
		float animationTime;
	};

}