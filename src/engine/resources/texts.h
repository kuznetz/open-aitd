#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "config.h"
#include "data_path.h"

using namespace std;
namespace openAITD {

	class Texts {
	public:
	 	static inline const std::string defaultLanguage = "en";
	   bool loaded = false;
		map<int,string> texts;
		// Engine texts loaded from engine.txt, stored as "section.key" -> value.
		unordered_map<string,string> engine;
		Font mainFont;
		Config& config;

		static std::vector<std::string> getLanguageList() {
			return DataPath::ListDirs("texts");
		}

		Texts(Config& config):
		  config(config)
		  {}

		~Texts() {
			unload();
		}

		// Parses a codepoints file: hex values and ranges ("0410-042F", "0x0410-0x042F").
		// Separators are spaces and commas; comments run from '#' to the end of the line.
		static void parseCodepoints(const std::string& path, std::vector<int>& cp) {
			std::ifstream in(path);
			if (!in.is_open()) return;

			auto toInt = [](const std::string& s) {
				return (int)std::stoul(s, nullptr, 16); // base 16, accepts the 0x prefix
			};

			std::string line;
			while (std::getline(in, line)) {
				auto hash = line.find('#');
				if (hash != std::string::npos) line.erase(hash);
				for (char& ch : line) if (ch == ',') ch = ' ';

				std::istringstream ss(line);
				std::string tok;
				while (ss >> tok) {
					try {
						auto dash = tok.find('-');
						if (dash == std::string::npos) {
							cp.push_back(toInt(tok));
						} else {
							int a = toInt(tok.substr(0, dash));
							int b = toInt(tok.substr(dash + 1));
							if (a > b) std::swap(a, b);
							for (int c = a; c <= b; ++c) cp.push_back(c);
						}
					} catch (const std::exception&) {
						// skip malformed token
					}
				}
			}
		}

		// Codepoints for a language: the basic ASCII set plus language-specific ones
		// from texts/<language>/codepoints.txt, falling back to the default language.
		std::vector<int> getCodepoints(const std::string& language) {
			std::vector<int> cp;

			// Basic set: printable ASCII only, always present in any font.
			for (int c = 0x20; c <= 0x7E; ++c) cp.push_back(c);

			// Language-specific additions.
			std::string path = DataPath::GetFile("texts/" + language + "/codepoints.txt");
			if (path.empty() && language != defaultLanguage)
				path = DataPath::GetFile("texts/" + defaultLanguage + "/codepoints.txt");
			if (!path.empty()) parseCodepoints(path, cp);

			// Deduplicate and sort.
			std::sort(cp.begin(), cp.end());
			cp.erase(std::unique(cp.begin(), cp.end()), cp.end());
			return cp;
		}

    void load() {
      if (loaded) {
        unload();
      }
      texts.clear();
      engine.clear();
      string s = DataPath::GetFile("texts/" + defaultLanguage + "/main.txt");
      loadTexts(s);
      s = DataPath::GetFile("texts/" + config.language + "/main.txt");
      loadTexts(s);

      s = DataPath::GetFile("texts/" + defaultLanguage + "/engine.txt");
      loadEngineTexts(s);
      s = DataPath::GetFile("texts/" + config.language + "/engine.txt");
      loadEngineTexts(s);

			auto codepoints = getCodepoints(config.language);
			s = DataPath::GetFile("texts/" + config.language + "/font.ttf");
			if (s != "") {
				mainFont = LoadFontEx(s.c_str(), config.screenH * 16 / 200, codepoints.data(), (int)codepoints.size());
			} else {
				s = DataPath::GetFile("texts/" + defaultLanguage + "/font.ttf");
	  		mainFont = LoadFontEx(s.c_str(), config.screenH * 16 / 200, codepoints.data(), (int)codepoints.size());
			}

			loaded = true;
    }

    void unload() {
      if (!loaded) return;
      texts.clear();
      engine.clear();
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

    string getText(const int id) {
      if (!loaded) {
        load();
      }
      return texts[id];
    }

  static std::string trim(const std::string& s) {
   size_t a = s.find_first_not_of(" \t\r\n");
   if (a == std::string::npos) return "";
   size_t b = s.find_last_not_of(" \t\r\n");
   return s.substr(a, b - a + 1);
  }

  // Parses an engine texts file in the format:
  //   [section]
  //   key=value
  // Lines starting with '#' or ';' are treated as comments.
  // Keys are stored as "section.key".
  void loadEngineTexts(const string& textsPath) {
   if (textsPath.empty()) return;
   ifstream inFile(textsPath);
   if (!inFile.is_open()) return;

   string section;
   string str;
   while (getline(inFile, str)) {
    string line = trim(str);
    if (line.empty()) continue;
    if (line[0] == '#' || line[0] == ';') continue;

    // Section header: [section].
    if (line.front() == '[' && line.back() == ']') {
    	section = trim(line.substr(1, line.size() - 2));
    	continue;
    }

    auto eq = line.find('=');
    if (eq == string::npos) continue;

    string key = trim(line.substr(0, eq));
    string value = trim(line.substr(eq + 1));
    if (key.empty()) continue;

    engine[section.empty() ? key : section + "." + key] = value;
   }
  }

  // Returns the engine text for "section.key" (the fallback language is
  // loaded first, so the selected language overrides it).
  string getEngineText(const string& key) {
   if (!loaded) {
    load();
   }
   auto it = engine.find(key);
   return it != engine.end() ? it->second : "";
  }

	 string getBookText(const int textId) {
			string path = DataPath::GetFile("texts/" + config.language + "/" + to_string(textId + 1) + ".txt");
			std::ifstream file(path);
			if (file.is_open()) {
					std::ostringstream buffer;
					buffer << file.rdbuf();
					return buffer.str();
			}

			string defaultPath = DataPath::GetFile("texts/" + defaultLanguage + "/" + to_string(textId + 1) + ".txt");
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
