#pragma once
#include <vector>
#include <map>
#include <string>

#include "../raylib.h"
#include "config.h"
#include "stages.h"

using namespace std;
using namespace raylib;
namespace openAITD {

	struct BackgroundOverlay {
		raylib::Rectangle bounds;
		Texture2D texture;
	};

	struct Background {
		//Image background;
		Texture2D texture;
		//vector<vector<Image>> masks;
		vector<vector<BackgroundOverlay>> overlays;
	};

	class Backgrounds
	{
	public:
		Config* config = 0;
		vector<Stage>* stages = 0;
		string stageDir = "data/backgrounds";
		vector<Background> items;

		Backgrounds() {
		}

		~Backgrounds() {
			clear();
		}

		void clear() {
			for (int i = 0; i < items.size(); i++) {
				UnloadTexture(items[i].texture);
				for (int j = 0; j < items[i].overlays.size(); j++) {
					for (int k = 0; k < items[i].overlays[j].size(); k++) {
						UnloadTexture(items[i].overlays[j][k].texture);
					}
				}
			}
			items.clear();
		}

		void loadStage(int newStageId)
		{
			if (newStageId == curStageId) return;
			clear();
			auto curStage = &(*this->stages)[newStageId];
			for (int camId = 0; camId < curStage->cameras.size(); camId++) {
				auto dir = curStage->stageDir + "/camera_" + to_string(camId);

				auto& bg = items.emplace_back();

				auto bgImg = raylib::LoadImage((dir + "/background.png").c_str());
				auto& fullBgImg = resizeImg(bgImg, config->screenW, config->screenH);
				UnloadImage(bgImg);

				bg.texture = LoadTextureFromImage(fullBgImg);

				auto& camRooms = curStage->cameras[camId].rooms;
				bg.overlays.resize(camRooms.size());
				for (int cr = 0; cr < camRooms.size(); cr++) {
					bg.overlays[cr].resize(camRooms[cr].overlays.size());
					for (int ovlId = 0; ovlId < camRooms[cr].overlays.size(); ovlId++) {
						auto mskImg = raylib::LoadImage((dir + "/mask_" + to_string(cr) + "_" + to_string(ovlId) + ".png").c_str());
						bg.overlays[cr][ovlId] = generateOverlay(fullBgImg, mskImg);
						UnloadImage(mskImg);
					}
				}

				UnloadImage(fullBgImg);
			}
			curStageId = newStageId;
		}

		Background* get(int stageId, int roomId) {
			loadStage(stageId);
			return &items[roomId];
		}

	private:
		int curStageId  = -1;

		Image resizeImg(Image& src, int w, int h) {
			Image res = { 0, w, h, 1, src.format };
			stbir_pixel_layout l = STBIR_RGB;
			if (src.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) l = STBIR_RGBA;
			if (src.format == RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA) l = STBIR_RA;
			res.data = stbir_resize_uint8_linear((unsigned char*)src.data, src.width, src.height, 0, 0, w, h, 0, l);
			return res;
		}

		BackgroundOverlay generateOverlay(Image& fullBg, Image& mask) {
			Image maskImageScaled = resizeImg(mask, config->screenW, config->screenH);
			Image resImg = { 0, config->screenW, config->screenH, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
			int pixelCount = config->screenW * config->screenH;
			resImg.data = new uint8_t[pixelCount * 4];
			for (int i = 0; i < pixelCount; i++) {
				((uint8_t*)resImg.data)[i * 4 + 0] = ((uint8_t*)fullBg.data)[i * 3 + 0];
				((uint8_t*)resImg.data)[i * 4 + 1] = ((uint8_t*)fullBg.data)[i * 3 + 1];
				((uint8_t*)resImg.data)[i * 4 + 2] = ((uint8_t*)fullBg.data)[i * 3 + 2];
				((uint8_t*)resImg.data)[i * 4 + 3] = ((uint8_t*)maskImageScaled.data)[i * 2 + 1];
			}
			Texture2D result = LoadTextureFromImage(resImg);
			UnloadImage(maskImageScaled);
			UnloadImage(resImg);
			return { {}, result };
		}

	};

}