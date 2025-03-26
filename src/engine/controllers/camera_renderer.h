#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"

using namespace std;
namespace openAITD {
	#include <raylib.h>

	class Background {
	public:
		Image background;
		vector<vector<Image>> masks;

		Background(Stage* stage, int cameraId) {
			auto dir = stage->stageDir + "/camera_" + to_string(cameraId);
			background = LoadImage((dir+"/background.png").c_str());
			auto& camRooms = stage->cameras[cameraId].rooms;
			for (int cr = 0; cr < camRooms.size(); cr++) {
				auto& msk = masks.emplace_back();
				for (int i = 0; i < camRooms[cr].overlays.size(); i++) {
					auto& o = camRooms[cr].overlays[i];
					msk.push_back(LoadImage((dir + "/mask_" + to_string(camRooms[cr].roomId) + "_" + to_string(i) + ".png").c_str()));
				}
			}
		}

		~Background() {
			UnloadImage(background);
			for (int i = 0; i < masks.size(); i++) {
				for (int j = 0; j < masks[i].size(); j++) {
					UnloadImage(masks[i][j]);
				}
			}
		}
	};

	class CameraRenderer {
	public:
		World* world;
		Resources* resources;

		vector<Background> backgrounds;
		int bgTexture;
		vector<vector<int>> overlayTextures;

		Stage* curStage;
		int curStageId = -1;
		GCamera* curCamera;
		int curCameraId = -1;

		CameraRenderer(Resources* res, World* world) {
			this->resources = res;
			this->world = world;
		}

		void clearStage()
		{
			backgrounds.clear();
		}

		void loadStage(int stage)
		{
			if (curStageId == -1) return;
			if (stage == curStageId) return;
			clearStage();
			curStage = &this->resources->stages[stage];
			for (int i = 0; i < curStage->cameras.size(); i++ ) {
				backgrounds.emplace_back(curStage, i);
			}
			curStageId = stage;
			curCameraId = -1;
		}

		void setCamera(int stage, int camera)
		{
			loadStage(stage);
			//TODO: make bg textures
			curCameraId = camera;
		}
	};

}