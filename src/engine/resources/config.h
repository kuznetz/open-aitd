#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <exception>

#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>
using namespace nlohmann;

namespace openAITD {

	std::string configPath("data/config.json");

	struct Config
	{
		bool showFps = false;
		bool fulllscreen = true;
		int screenX = 0;
		int screenY = 0;
		int screenW = 1280;
		int screenH = 960;
		int targetFps = 60;
		float antialiasing = 2.0f;
	};

	Config loadConfig() {
		Config cfg;
		if (!std::filesystem::exists(configPath)) return cfg;
		std::ifstream ifs(configPath);
		json confJson = json::parse(ifs);
		if (confJson.contains("fulllscreen")) {
			cfg.fulllscreen = confJson["fulllscreen"];
		}
		if (confJson.contains("antialiasing")) {
			cfg.antialiasing = confJson["antialiasing"];
		}
		if (!cfg.fulllscreen) {
			cfg.screenW = 320;
			cfg.screenH = 240;
			try {
				cfg.screenW = confJson["screenW"];
				cfg.screenH = confJson["screenH"];
			} catch (std::exception e) {
			}
		}
		else {
			cfg.screenW = 1280;
			cfg.screenH = 960;
		}

		cfg.showFps = confJson["showFps"] || false;
		cfg.targetFps = 144;
		return cfg;
	}

	void saveConfig(Config cfg) {
		json outJson = json::object();

		outJson["fulllscreen"] = cfg.fulllscreen;
		outJson["antialiasing"] = cfg.antialiasing;
		outJson["screenW"] = cfg.screenW;
		outJson["screenH"] = cfg.screenH;
		outJson["showFps"] = cfg.showFps;

		std::ofstream o(configPath);
		o << std::setw(2) << outJson << std::endl;
	}

}
