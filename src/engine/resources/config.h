#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <exception>

#include "../../platform/platform.h"

#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>
using namespace nlohmann;

namespace openAITD {

	std::string configPath("saves/config.json");

	// Gain value in [0.0, 1.0] for the Sound/Music/Voice settings.
	inline float clampVolume(float value) {
		if (value < 0.0f) return 0.0f;
		if (value > 1.0f) return 1.0f;
		return value;
	}

	struct Config
	{
		std::string language;
		bool fastStart = false;
		bool showFps = false;
		bool fulllscreen = true;
		int screenX = 0;
		int screenY = 0;
		int screenW = 640;
		int screenH = 480;
		int targetFps = 60;
		float antialiasing = 2.0f;
		// Volume gain values in [0.0, 1.0] for sound effects/music/voice.
		float soundVolume = 0.6f;
		float musicVolume = 0.6f;
		float voiceVolume = 0.6f;
	};

	Config loadConfig() {
		Config cfg;
		if (!std::filesystem::exists(configPath)) return cfg;
		std::ifstream ifs(configPath);
		json confJson = json::parse(ifs);
		if (confJson.contains("language")) {
      cfg.language = confJson["language"];
		} else {
      cfg.language = getSystemLanguageCode();
		}
		if (confJson.contains("fastStart")) {
			cfg.fastStart = confJson["fastStart"];
		}
		if (confJson.contains("fulllscreen")) {
			cfg.fulllscreen = confJson["fulllscreen"];
		}
		if (confJson.contains("antialiasing")) {
			cfg.antialiasing = confJson["antialiasing"];
		}
		if (!cfg.fulllscreen) {
			try {
				cfg.screenW = confJson["screenW"];
				cfg.screenH = confJson["screenH"];
			} catch (std::exception e) {
			}
		}

		if (confJson.contains("soundVolume")) {
			cfg.soundVolume = confJson["soundVolume"];
		}
		if (confJson.contains("musicVolume")) {
			cfg.musicVolume = confJson["musicVolume"];
		}
		if (confJson.contains("voiceVolume")) {
			cfg.voiceVolume = confJson["voiceVolume"];
		}
		cfg.soundVolume = clampVolume(cfg.soundVolume);
		cfg.musicVolume = clampVolume(cfg.musicVolume);
		cfg.voiceVolume = clampVolume(cfg.voiceVolume);

		cfg.showFps = confJson["showFps"] || false;
		cfg.targetFps = 144;
		return cfg;
	}

	void saveConfig(Config cfg) {
		json outJson = json::object();

    outJson["language"] = cfg.language;
		outJson["fastStart"] = cfg.fastStart;
		outJson["fulllscreen"] = cfg.fulllscreen;
		outJson["antialiasing"] = cfg.antialiasing;
		outJson["screenW"] = cfg.screenW;
		outJson["screenH"] = cfg.screenH;
		outJson["showFps"] = cfg.showFps;
		outJson["soundVolume"] = cfg.soundVolume;
		outJson["musicVolume"] = cfg.musicVolume;
		outJson["voiceVolume"] = cfg.voiceVolume;

		std::ofstream o(configPath);
		o << std::setw(2) << outJson << std::endl;
	}

}
