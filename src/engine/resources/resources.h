#pragma once
#include <vector>
#include <string>
#include <raymath.h>
#include "stages.h"

using namespace std;
namespace openAITD {

	//Store static data in game
	class Resources {
	public:
		vector<Stage> stages;
	};

}