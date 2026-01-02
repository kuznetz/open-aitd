#pragma once
#include <vector>
#include <string>
#include <cctype>
#include <sstream>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../raylib-cpp.h"

namespace openAITD {
  using namespace std;
  using namespace raylib;

	class ConsoleScreen {
	public:
		World* world;
		Resources* resources;
    string curInput;
    bool firstFrame = true;
    int lineHeight = 0;
    int fontSize = 20;
    int curInfo = -1;
		bool exit;
    string lines[20];

		ConsoleScreen(World* world) {
			this->world = world;
			this->resources = world->resources;
      lineHeight = fontSize;
		}

		~ConsoleScreen() {
		}

		void RenderLines() {
			raylib::DrawText("CONSOLE", 10, 10, fontSize, GREEN);

      int curY = resources->config.screenH - (10 + lineHeight);
			raylib::DrawText((curInput+"_").c_str(), 10, curY, fontSize, WHITE);
      curY -= lineHeight * 2;

      for (int i = 19; i >= 0; i--) {
        if (lines[i].length() > 0) {
          raylib::DrawText(lines[i].c_str(), 10, curY, fontSize, LIGHTGRAY);
          curY -= lineHeight;
        }
      }
		}

    std::vector<std::string> SplitBySpace(const std::string& str) {
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = str.find(' ');
        
        while (end != std::string::npos) {
            if (end != start) {  // Avoid empty tokens from consecutive spaces
                tokens.push_back(str.substr(start, end - start));
            }
            start = end + 1;
            end = str.find(' ', start);
        }
        
        // Add the last token
        if (start < str.length()) {
            tokens.push_back(str.substr(start));
        }
        
        return tokens;
    }    

		void Submit() {
      ClearLines();
      auto tokens = SplitBySpace(curInput);
      if (tokens.size() == 0) {
        ShowHelp();
        return;
      }
      char cmd = toupper(tokens[0][0], std::locale::classic());
      if (cmd == 'J') {
        lines[0] = "JUMP!";
      } else if (cmd == 'T') {
        lines[0] = "TAKE!";
      } else if (cmd == 'O') {
        ShowObjectInfo(tokens);
      } else {
        ShowHelp();
      }
      curInput = "";
		}

    void ClearLines() {
      for (int i = 0; i < 20; i++) {
        lines[i] = "";
      }
    }

    void ShowHelp() {
      lines[1] = "Commands: ";
      lines[2] = "J {STAGE} {ROOM} - Jump to room";
      lines[3] = "T {OBJECT_ID} - Take item ";
      lines[4] = "O {OBJECT_ID} - Object info";
      //lines[4] = "[H]eal";
    }

    void ShowObjectInfo(const vector<string>& tokens) {
      if (tokens.size() != 2) {
        lines[0] = "Invalid arguments";
        ShowHelp();
        return;
      }
      try {
          int objId = std::stoi(tokens[1]);
          ShowObjectInfo2(objId);
      }
      catch (const std::exception& e) {
          lines[0] = "Invalid object ID";
          ShowHelp();
      }
    }

    void ShowObjectInfo2(int objId) {
      curInfo = objId;
      try {

        auto& gobj = world->gobjects[objId];
        stringstream ss;
        ss << "Object " << objId << ":";
        lines[0] = ss.str();
        ss.str("");

        auto& loc = gobj.location;
        ss << "Stage:" << loc.stageId << " Room:" << loc.roomId;
        lines[1] = ss.str();
        ss.str("");

        ss << "X:" << loc.position.x << " Y:" << loc.position.y << " Z:" << loc.position.z;
        lines[2] = ss.str();
        ss.str("");

        ss << 
          "Model: " << gobj.modelId << 
          " Anim: " << gobj.animation.animIdx;
        if (gobj.animation.bitField.repeat) ss << " Rep";
        if (gobj.animation.bitField.reset) ss << " Rst";
        if (gobj.animation.bitField.uninterruptable) ss << " UnInt";
        lines[3] = ss.str();
        ss.str("");

        ss << "Life: " << (int)gobj.lifeMode << " " << gobj.lifeId;
        lines[3] = ss.str();
        ss.str("");     

      } catch (exception e) {
        lines[0] = "Invalid object ID";
        ShowHelp();
      }
    }

		void ProcessKeys() {
			if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_GRAVE)) {
				exit = true;
  			return;
			}
			if (IsKeyPressed(KEY_ENTER)) {
  			Submit();
			}
      // || IsKeyPressed(KEY_SPACE)
			if (IsKeyPressed(KEY_UP)) {
			}
			if (IsKeyPressed(KEY_DOWN)) {
			}
			if (IsKeyPressed(KEY_BACKSPACE)) {
        if (curInput.length() > 0) {
          curInput.pop_back();
        }
			}      
      int inpChar = raylib::GetCharPressed(); 
      if (inpChar && inpChar < 128) {
        curInput = curInput + (char)inpChar;
      }
		}

		void start() {
			firstFrame = true;
      exit = false;
			float timeDelta;

      if (curInfo >=0 ) {
        ShowObjectInfo2(curInfo);
      } else {
        ShowHelp();
      }      

      while (!exit) {
        if (firstFrame) {
					firstFrame = false;
				} else {
          timeDelta = GetFrameTime();
          ProcessKeys();
        }
				resources->screen.begin();
        RenderLines();
  			resources->screen.end();
			}      
		}

	};

}