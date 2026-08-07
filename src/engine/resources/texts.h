#pragma once

#include <vector>
#include <map>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>

#include "config.h"

using namespace std;
namespace openAITD {

	class Texts {
	public:
  	static inline const std::string defaultLanguage = "en";
    bool loaded = false;
    string language = "en";
		map<int,string> texts;
		Font mainFont;
		Config& config;

		Texts(Config& config):
		  config(config)
		  {}

		~Texts() {
			unload();
		}

    void load() {
			string s = "data/texts/" + defaultLanguage + "/main.txt";
			loadTexts(s);
      s = "data/texts/" + language + "/main.txt";
      loadTexts(s);

			s = DataPath::GetFile("texts/" + language + "/font.ttf");
			if (s != "") {
				mainFont = LoadFontEx(s.c_str(), config.screenH * 16 / 200, 0, 95);
			} else {
				s = DataPath::GetFile("texts/" + defaultLanguage + "/font.ttf");
	  		mainFont = LoadFontEx(s.c_str(), config.screenH * 16 / 200, 0, 95);
			}
    }

    void unload() {
			if (!loaded) return;
			UnloadFont(mainFont);
			loaded=false;
		}

		void loadTexts(string textsPath) {
			int idx;
			string str;
			ifstream inFile;
			inFile.open(textsPath);
			while (getline(inFile, str))
			{
				if (str[0] != '@') continue;
				idx = 0;
				int i = 1;
				while (str[i] >= '0' && str[i] <= '9') // parse string number
				{
					idx = idx * 10 + (str[i] - 48);
					i++;
				}
				if (str[i] == ':') // start of string
				{
					texts[idx] = str.substr(i+1);
				}
			}
		}

    void setLanguage(const string lang) {
      language = lang;
      loaded = false;
    }

    string getText(const int id) {
      if (!loaded) {
        load();
      }
      return texts[id];
    }

		string getBookText(const int textId) {
			string path = "data/texts/" + language + "/" + to_string(textId + 1) + ".txt";
			std::ifstream file(path);
			if (file.is_open()) {
					std::ostringstream buffer;
					buffer << file.rdbuf();
					return buffer.str();
			}

			string defaultPath = "data/texts/" + defaultLanguage + "/" + to_string(textId + 1) + ".txt";
			std::ifstream defaultFile(defaultPath);
			if (defaultFile.is_open()) {
					std::ostringstream buffer;
					buffer << defaultFile.rdbuf();
					return buffer.str();
			}

			throw std::runtime_error("Read error: " + path + " and " + defaultPath);
		}

		void drawLeft(const char* text, raylib::Rectangle r, Color color) {
				auto& f = this->mainFont;
				Vector2 v = { r.x, r.y };
				DrawTextEx(f, text, v, f.baseSize, 0, color);
		}

		void drawCentered(const char* text, raylib::Rectangle r, Color color) {
				auto& f = this->mainFont;
				Vector2 mt = MeasureTextEx(f, text, f.baseSize, 0);
				int x = (int)(r.x + ((r.width - mt.x) / 2));
				Vector2 v = { (float)x, r.y };
				DrawTextEx(f, text, v, f.baseSize, 0, color);
		}

	};

}