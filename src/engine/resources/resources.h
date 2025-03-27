#pragma once
#include <vector>
#include <string>
#include "stages.h"
#include "models.h"
#include "../raylib.h"

using namespace std;
namespace openAITD {

	//Store static data in game
	class Resources {
	public:
		vector<Stage> stages;
		RModels models;
	};

}