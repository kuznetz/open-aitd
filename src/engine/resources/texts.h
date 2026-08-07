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

		std::vector<int> getCodepoints() {
				std::vector<int> cp;
				for (int c = 32; c <= 255; ++c) cp.push_back(c);
				for (int c = 0x0080; c <= 0x00FF; ++c) cp.push_back(c);				
				for (int с = 0x0410; с <= 0x042F; ++с) cp.push_back(с);
				for (int с = 0x0430; с <= 0x044F; ++с) cp.push_back(с);
				cp.push_back(0x0401);
				cp.push_back(0x0451);

				static const int aitd1_codes[] = {
						0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
						0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
						0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
						0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x003F, 0x00A7,
						0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
						0x00BF, 0x00A9, 0x00AA, 0x00BD, 0x00BC, 0x00E4, 0x00AE, 0x0069,
						0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
						0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
						0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
						0x00C9, 0x00C8, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
						0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7,
						0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
						0x00DE, 0x00DF, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
						0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
						0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7,
						0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00B2, 0x00FE, 0x00FF
				};
				for (int code : aitd1_codes) cp.push_back(code);				

  		return cp;
		}

    void load() {
			string s = "data/texts/" + defaultLanguage + "/main.txt";
			loadTexts(s);
      s = "data/texts/" + language + "/main.txt";
      loadTexts(s);

			auto& codepoints = getCodepoints();
			s = DataPath::GetFile("texts/" + language + "/font.ttf");
			if (s != "") {
				mainFont = LoadFontEx(s.c_str(), config.screenH * 16 / 200, codepoints.data(), codepoints.size());
			} else {
				s = DataPath::GetFile("texts/" + defaultLanguage + "/font.ttf");
	  		mainFont = LoadFontEx(s.c_str(), config.screenH * 16 / 200, codepoints.data(), codepoints.size());
			}

			loaded = true;
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