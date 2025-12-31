#pragma once
#include <vector>
#include <map>
#include <string>

#include "../raylib-cpp.h"
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

	struct IntRect {
		int x;
		int y;
		int width;
		int height;
	};

	class Backgrounds
	{
	public:
		Config* config = 0;
		vector<Stage>* stages = 0;
		string stageDir = "data/stages";
		string newStageDir = "newdata/stages";
		vector<Background> items;

		int curPictureId = -1;
		string picturesDir = "data/pictures";
		string newPicturesDir = "newdata/pictures";
		raylib::Texture2D picture = { 0 };

		Backgrounds() {
		}

		~Backgrounds() {
			clear();
			UnloadTexture(picture);
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

		Texture2D loadPicture(int pictureId) {
			if (curPictureId == pictureId) return picture;
			UnloadTexture(picture);
			
			auto imgS = newPicturesDir + "/" + to_string(pictureId) + ".png";
			if (!filesystem::exists(imgS)) {
				imgS = picturesDir + "/" + to_string(pictureId) + ".png";
			}
			if (!filesystem::exists(imgS)) {
				throw new exception((imgS+" not exists").c_str());
			}			
			
			auto bgImg = raylib::LoadImage(imgS.c_str());
			auto& fullBgImg = resizeImg(bgImg, config->screenW, config->screenH);
			UnloadImage(bgImg);
			picture = LoadTextureFromImage(fullBgImg);
			curPictureId = pictureId;
			return picture;
		}

		string getImgStagePath(string tail) {
			string path = newStageDir + "/" + tail;
			if (filesystem::exists(path)) {
				return path;
			}
			path = stageDir + "/" + tail;
			if (filesystem::exists(path)) {
				return path;
			}
			throw new exception((path + " not exists").c_str());
		}

		void loadStage(int newStageId)
		{
			if (newStageId == curStageId) return;
			clear();
			auto curStage = &(*this->stages)[newStageId];
			for (int camId = 0; camId < curStage->cameras.size(); camId++) {
				auto cameraDir = to_string(newStageId) + "/camera_" + to_string(camId);
				auto path = getImgStagePath(cameraDir + "/background.png");

				auto& bg = items.emplace_back();
				auto bgImg = raylib::LoadImage(path.c_str());
				auto& fullBgImg = resizeImg(bgImg, config->screenW, config->screenH);
				UnloadImage(bgImg);
				bg.texture = LoadTextureFromImage(fullBgImg);

				auto& camRooms = curStage->cameras[camId].rooms;
				bg.overlays.resize(camRooms.size());
				for (int cr = 0; cr < camRooms.size(); cr++) {
					bg.overlays[cr].resize(camRooms[cr].overlays.size());
					for (int ovlId = 0; ovlId < camRooms[cr].overlays.size(); ovlId++) {
						path = getImgStagePath(cameraDir + "/mask_" + to_string(cr) + "_" + to_string(ovlId) + ".png");
						auto mskImg = raylib::LoadImage(path.c_str());
						bg.overlays[cr][ovlId] = generateOverlayMask(fullBgImg, mskImg);
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

		IntRect findImageBounds(const Image& image) {
			int left = image.width;
			int top = image.height;
			int right = -1;
			int bottom = -1;
			uint8_t px;
			for (int y = 0; y < image.height; ++y) {
				for (int x = 0; x < image.width; ++x) {
					px = ((uint8_t*)image.data)[(y * image.width + x) * 2 + 1];
					if (px != 0) {						
						left = std::min(left, x);
						right = std::max(right, x);
						top = std::min(top, y);
						bottom = std::max(bottom, y);
					}
				}
			}
			// If no non-background pixels found, return empty bounds
			if (right == -1) {
				return { 0, 0, 0, 0 };
			}
			return { left, top, (right - left), (bottom - top) };
		}

		BackgroundOverlay generateOverlayMask(Image& fullBg, Image& mask) {
			Image maskImageScaled = resizeImg(mask, config->screenW, config->screenH);
			IntRect rect = findImageBounds(maskImageScaled);

			Image resImg = { 0, rect.width, rect.height, 1, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE };
			int pixelCount = resImg.width * resImg.height;
			resImg.data = new uint8_t[pixelCount];

			int oriIdx;
			for (int y = 0; y < rect.height; y++) {
				for (int x = 0; x < rect.width; x++) {
					oriIdx = (y + rect.y) * config->screenW + x + rect.x;
					((uint8_t*)resImg.data)[ y * rect.width + x ] = 
						((uint8_t*)maskImageScaled.data)[oriIdx * 2 + 1];
				}
			}

			

			Texture2D result = LoadTextureFromImage(resImg);
			UnloadImage(maskImageScaled);
			UnloadImage(resImg);
			return { { (float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height }, result };
		}

		BackgroundOverlay generateOverlayColored(Image& fullBg, Image& mask) {
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