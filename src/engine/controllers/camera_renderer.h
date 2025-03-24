#pragma once
#include "../world/world.h"

using namespace std;
namespace openAITD {

	class CameraRenderer {
	public:
		World* world;
		int curStage;
		int curCamera;

		void setCamera(int stage, int camera);
		void render();
	};

	void Camera::setCamera(int stage, int camera)
	{

	}

	void Camera::render()
	{

	}

}