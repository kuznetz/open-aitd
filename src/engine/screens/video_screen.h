#pragma once
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <ogg/ogg.h>
#include <theora/theoradec.h>

#include "../world/world.h"
#include "../resources/resources.h"
#include "../resources/data_path.h"
#include "../../common/raylib_cpp.hpp"

using namespace std;
using namespace raylib;

namespace openAITD {

	// Plays an .ogv (Ogg/Theora) file - video only, no audio.
	//
	// Theora delivers I420 (YUV 4:2:0) planes; instead of converting YUV->RGB on
	// the CPU, the three planes are uploaded into three single-channel textures and
	// the conversion is done on the GPU by an inline GLSL 330 shader (see docs/video.md).
	//
	// Playback is driven by process(timeDelta) and drawn into the engine's scene
	// render texture, so it composes with the rest of the screens.
	class VideoScreen {
	public:
		enum class Status { Idle, Playing, Finished, Error };

		World* world;
		Resources* resources;

		Status status = Status::Idle;
		string curPath;
		bool finished = false;   // set once the whole clip has been shown

		VideoScreen(World* world)
			: world(world)
			, resources(world->resources)
		{}

		~VideoScreen() {
			stop();
		}

		// Opens a video file and prepares decoding/GPU resources.
		// `path` may be a direct file path or a DataPath-relative name (videos/x.ogv).
		bool start(const string& path) {
			stop();

			string resolved = resolvePath(path);
			if (resolved.empty()) {
				TraceLog(LOG_ERROR, "VideoScreen: video file not found: %s", path.c_str());
				status = Status::Error;
				return false;
			}

			file = fopen(resolved.c_str(), "rb");
			if (!file) {
				TraceLog(LOG_ERROR, "VideoScreen: cannot open: %s", resolved.c_str());
				status = Status::Error;
				return false;
			}
			curPath = resolved;

			ogg_sync_init(&sync);
			syncInit = true;
			th_info_init(&info);
			th_comment_init(&comment);

			if (!readHeaders()) {
				TraceLog(LOG_ERROR, "VideoScreen: failed to read Theora headers: %s", resolved.c_str());
				stop();
				status = Status::Error;
				return false;
			}

			fps = (info.fps_denominator > 0)
				? (double)info.fps_numerator / (double)info.fps_denominator
				: 25.0;
			if (fps <= 0.0) fps = 25.0;
			frameDuration = 1.0f / (float)fps;

			if (!createTextures()) {
				stop();
				status = Status::Error;
				return false;
			}

			ensureShaderLoaded();
			if (status == Status::Error) {
				stop();
				status = Status::Error;
				return false;
			}
			bindSamplers();

			curTime = 0;
			finished = false;
			hasFrame = false;
			hasPendingFrame = false;
			eofReached = false;
			lastFrameTime = 0.0;
			status = Status::Playing;
			return true;
		}

		// Releases every resource; safe to call repeatedly / from the destructor.
		void stop() {
			if (file) { fclose(file); file = nullptr; }
			if (decoder) { th_decode_free(decoder); decoder = nullptr; }
			if (setup) { th_setup_free(setup); setup = nullptr; }
			if (streamInit) { ogg_stream_clear(&stream); streamInit = false; }
			if (syncInit) { ogg_sync_clear(&sync); syncInit = false; }
			th_info_clear(&info);
			th_comment_clear(&comment);

			unloadTextures();
			if (yuvShader.id != 0) {
				UnloadShader(yuvShader);
				yuvShader = { 0 };
			}
			locY = locU = locV = -1;
			shaderTried = false;

			packedY.clear();
			packedU.clear();
			packedV.clear();

			curPath.clear();
			curTime = 0;
			savedDataPacket = false;
			hasFrame = false;
			hasPendingFrame = false;
			eofReached = false;
			lastFrameTime = 0.0;
			status = Status::Idle;
		}

		bool isFinished() const { return finished || status == Status::Finished; }

		// Advances the clip clock and uploads every frame whose presentation time
		// has been reached.
		void process(float timeDelta) {
			if (status != Status::Playing) return;
			curTime += timeDelta;

			int uploaded = 0;
			while (uploaded < maxFramesPerTick) {
				if (!hasPendingFrame) {
					if (eofReached || !decodeNextFrame()) {
						eofReached = true;
						if (!hasFrame || curTime >= lastFrameTime) {
							finished = true;
							status = Status::Finished;
						}
						return;
					}
					hasPendingFrame = true;
				}
				// The very first decoded frame is shown immediately.
				if (hasFrame && frameTime > curTime) break;
				uploadFrame();
				hasFrame = true;
				lastFrameTime = frameTime;
				hasPendingFrame = false;
				uploaded++;
			}

			if (eofReached && !hasPendingFrame && curTime >= lastFrameTime) {
				finished = true;
				status = Status::Finished;
			}
		}

		// Draws the current frame (aspect-fit, letterboxed) into sceneTex.
		void render() {
			if (!hasFrame || yTex.id == 0) return;

			float screenW = (float)resources->config.screenW;
			float screenH = (float)resources->config.screenH;

			float srcW = (float)(info.pic_width > 0 ? info.pic_width : info.frame_width);
			float srcH = (float)(info.pic_height > 0 ? info.pic_height : info.frame_height);
			float srcX = (float)info.pic_x;
			float srcY = (float)info.pic_y;

			float scale = (srcW > 0 && srcH > 0)
				? min(screenW / srcW, screenH / srcH)
				: 1.0f;
			float dstW = srcW * scale;
			float dstH = srcH * scale;
			float dstX = (screenW - dstW) * 0.5f;
			float dstY = (screenH - dstH) * 0.5f;

			BeginTextureMode(resources->screen.sceneTex);
			ClearBackground(BLACK);
			bindPlanes();
			BeginShaderMode(yuvShader);
			DrawTexturePro(
				yTex,
				{ srcX, srcY, srcW, srcH },
				{ dstX, dstY, dstW, dstH },
				{ 0, 0 }, 0, WHITE
			);
			EndShaderMode();
			EndTextureMode();
		}

	private:
		static constexpr int maxFramesPerTick = 4;
		static constexpr int readChunk = 8192;

		// --- file / demuxer ---
		FILE* file = nullptr;
		ogg_sync_state sync{};
		bool syncInit = false;
		ogg_stream_state stream{};
		bool streamInit = false;
		ogg_packet packet{};
		bool savedDataPacket = false;

		// --- decoder ---
		th_info info{};
		th_comment comment{};
		th_setup_info* setup = nullptr;
		th_dec_ctx* decoder = nullptr;

		// --- clip timing ---
		double fps = 25.0;
		float frameDuration = 0.04f;
		float curTime = 0.0f;
		double frameTime = 0.0;      // presentation time of the pending frame
		double lastFrameTime = 0.0;  // presentation time of the shown frame
		bool hasFrame = false;
		bool hasPendingFrame = false;
		bool eofReached = false;

		// --- GPU ---
		Texture2D yTex{}, uTex{}, vTex{};
		Shader yuvShader{};
		int locY = -1, locU = -1, locV = -1;
		bool shaderTried = false;

		// --- tightly packed staging buffers (used when stride != width) ---
		vector<uint8_t> packedY, packedU, packedV;

		th_ycbcr_buffer ycbcr{};

		static constexpr const char* vertexShader = R"(
			#version 330
			in vec3 vertexPosition;
			in vec2 vertexTexCoord;
			in vec4 vertexColor;
			uniform mat4 mvp;
			out vec2 fragTexCoord;
			out vec4 fragColor;
			void main() {
				fragTexCoord = vertexTexCoord;
				fragColor = vertexColor;
				gl_Position = mvp * vec4(vertexPosition, 1.0);
			}
		)";

		static constexpr const char* fragmentShader = R"(
			#version 330
			in vec2 fragTexCoord;
			in vec4 fragColor;
			uniform sampler2D texture0; // Y
			uniform sampler2D texture1; // U
			uniform sampler2D texture2; // V
			out vec4 finalColor;
			void main() {
				float y = texture(texture0, fragTexCoord).r;
				float u = texture(texture1, fragTexCoord).r - 0.5;
				float v = texture(texture2, fragTexCoord).r - 0.5;
				// BT.601 with headroom/footroom (see docs/video.md)
				y = 1.1643 * (y - 0.0625);
				float r = y + 1.5958 * v;
				float g = y - 0.39173 * u - 0.81290 * v;
				float b = y + 2.017 * u;
				finalColor = vec4(r, g, b, 1.0);
			}
		)";

		string resolvePath(const string& path) {
			FILE* f = fopen(path.c_str(), "rb");
			if (f) { fclose(f); return path; }

			string p = DataPath::GetFile(path);
			if (!p.empty()) return p;

			bool hasOgvExt = path.size() >= 4 &&
				path.compare(path.size() - 4, 4, ".ogv") == 0;
			if (!hasOgvExt) {
				p = DataPath::GetFile(path + ".ogv");
				if (!p.empty()) return p;
			}
			return "";
		}

		bool fillFromFile() {
			char* buffer = ogg_sync_buffer(&sync, readChunk);
			if (!buffer) return false;
			size_t bytes = fread(buffer, 1, readChunk, file);
			if (bytes == 0) return false;
			ogg_sync_wrote(&sync, (long)bytes);
			return true;
		}

		// Pulls the next packet of the Theora logical stream (audio is skipped).
		bool nextPacket(ogg_packet& pkt) {
			ogg_page page;
			while (true) {
				int r = streamInit ? ogg_stream_packetout(&stream, &pkt) : 0;
				if (r > 0) return true;

				while (true) {
					int pr = ogg_sync_pageout(&sync, &page);
					if (pr == 1) break;
					if (pr < 0) continue;         // stream desync, keep going
					if (!fillFromFile()) return false;  // EOF
				}

				if (!streamInit) {
					ogg_stream_init(&stream, ogg_page_serialno(&page));
					streamInit = true;
				}
				if (ogg_page_serialno(&page) != stream.serialno) continue;
				ogg_stream_pagein(&stream, &page);
			}
		}

		bool readHeaders() {
			while (true) {
				if (!nextPacket(packet)) return false;
				int ret = th_decode_headerin(&info, &comment, &setup, &packet);
				if (ret == 0) {
					// `packet` now holds the first video data packet: keep it for decoding.
					savedDataPacket = true;
					break;
				}
				if (ret < 0) {
					TraceLog(LOG_ERROR, "VideoScreen: not a Theora stream (error %d)", ret);
					return false;
				}
			}

			if (info.pixel_fmt != TH_PF_420) {
				TraceLog(LOG_ERROR,
					"VideoScreen: unsupported pixel format %d (only I420 is supported)",
					(int)info.pixel_fmt);
				return false;
			}

			decoder = th_decode_alloc(&info, setup);
			if (!decoder) {
				TraceLog(LOG_ERROR, "VideoScreen: th_decode_alloc failed");
				return false;
			}
			return true;
		}

		// Decodes packets until a frame (or a duplicate of the previous one) is ready.
		bool decodeNextFrame() {
			while (true) {
				if (savedDataPacket) {
					savedDataPacket = false;
				} else {
					if (!nextPacket(packet)) return false;
				}

				ogg_int64_t granulepos = -1;
				int ret = th_decode_packetin(decoder, &packet, &granulepos);
				if (ret < 0) continue;  // bad/skipped packet

				double t = th_granule_time(decoder, granulepos);
				double when = (t > 0.0 || granulepos == 0) ? t : lastFrameTime + frameDuration;

				if (ret == TH_DUPFRAME) {
					if (!hasFrame) continue;  // nothing to duplicate yet
					frameTime = when;
					return true;
				}

				if (th_decode_ycbcr_out(decoder, ycbcr) != 0) continue;
				frameTime = when;
				return true;
			}
		}

		static Texture2D createGrayTexture(int w, int h) {
			Image img = { 0 };
			img.width = w;
			img.height = h;
			img.mipmaps = 1;
			img.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
			img.data = calloc((size_t)w * (size_t)h, 1);
			Texture2D tex = LoadTextureFromImage(img);
			free(img.data);
			if (tex.id != 0) {
				SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
			}
			return tex;
		}

		bool createTextures() {
			int w = info.frame_width;
			int h = info.frame_height;
			if (w <= 0 || h <= 0) return false;

			int cw = (w + 1) / 2;
			int ch = (h + 1) / 2;

			yTex = createGrayTexture(w, h);
			uTex = createGrayTexture(cw, ch);
			vTex = createGrayTexture(cw, ch);

			if (yTex.id == 0 || uTex.id == 0 || vTex.id == 0) {
				TraceLog(LOG_ERROR, "VideoScreen: failed to create YUV textures");
				return false;
			}
			return true;
		}

		void unloadTextures() {
			if (yTex.id != 0) { UnloadTexture(yTex); yTex = { 0 }; }
			if (uTex.id != 0) { UnloadTexture(uTex); uTex = { 0 }; }
			if (vTex.id != 0) { UnloadTexture(vTex); vTex = { 0 }; }
		}

		void ensureShaderLoaded() {
			if (shaderTried) return;
			shaderTried = true;

			yuvShader = LoadShaderFromMemory(vertexShader, fragmentShader);
			if (yuvShader.id == 0) {
				TraceLog(LOG_ERROR, "VideoScreen: failed to build YUV->RGB shader");
				status = Status::Error;
				return;
			}
			locY = GetShaderLocation(yuvShader, "texture0");
			locU = GetShaderLocation(yuvShader, "texture1");
			locV = GetShaderLocation(yuvShader, "texture2");
		}

		// Pins the three samplers to fixed texture units (0/1/2) instead of relying
		// on SetShaderValueTexture, which raylib does not keep stable across draws.
		void bindSamplers() {
			if (yuvShader.id == 0) return;
			int yUnit = 0, uUnit = 1, vUnit = 2;
			if (locY != -1) SetShaderValue(yuvShader, locY, &yUnit, SHADER_UNIFORM_INT);
			if (locU != -1) SetShaderValue(yuvShader, locU, &uUnit, SHADER_UNIFORM_INT);
			if (locV != -1) SetShaderValue(yuvShader, locV, &vUnit, SHADER_UNIFORM_INT);
		}

		// Binds U/V to units 1/2 right before drawing; DrawTexturePro puts yTex on
		// unit 0 itself. Restores unit 0 as the active slot afterwards.
		void bindPlanes() {
			if (uTex.id != 0) {
				rlActiveTextureSlot(1);
				rlEnableTexture(uTex.id);
			}
			if (vTex.id != 0) {
				rlActiveTextureSlot(2);
				rlEnableTexture(vTex.id);
			}
			rlActiveTextureSlot(0);
		}

		static void uploadPlane(Texture2D& tex, const th_img_plane& plane,
			vector<uint8_t>& packed)
		{
			if (tex.id == 0 || plane.data == nullptr) return;

			const size_t rowBytes = (size_t)tex.width;
			if ((size_t)plane.stride == rowBytes) {
				UpdateTexture(tex, plane.data);
				return;
			}

			packed.resize(rowBytes * (size_t)tex.height);
			for (int row = 0; row < tex.height; row++) {
				memcpy(
					packed.data() + rowBytes * (size_t)row,
					plane.data + (size_t)plane.stride * (size_t)row,
					rowBytes
				);
			}
			UpdateTexture(tex, packed.data());
		}

		void uploadFrame() {
			uploadPlane(yTex, ycbcr[0], packedY);
			uploadPlane(uTex, ycbcr[1], packedU);
			uploadPlane(vTex, ycbcr[2], packedV);
		}
	};

}
