#pragma once
#include <vector>
#include <string>
#include <raymath.h>
#include "game_objects.h"

using namespace std;
namespace openAITD {

	struct Player {
		int objectId;
		bool hasControl;
		int animations[6];
	};

	//Store all dynamic data in game
	class World {
	public:
		vector<string> dataDirectories = { "./mods", "./oaitd", "./data" };
		vector<GameObject> gobjects;
		vector<short int> vars;
		vector<short int> cVars;
		//Object to follow camera
		int trackedGObjectId;
		int curCameraId;
		int curStageId;

		void loadGObjects(string path);
		void loadVars(string path);
		void setCharacter(bool alt) {}
	};

	void World::loadGObjects(string path)
	{

	};

	void World::loadVars(string path)
	{

	};

}