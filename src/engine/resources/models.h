#pragma once
#include <vector>
#include <map>
#include <string>

#include "../raylib.h"

using namespace std;
namespace openAITD {
	class RModel
	{
	public:
		Model* model;
		map<int,ModelAnimation> animations;
		BoundingBox bounds;

		void load(string modelDir) {
		
		}
	};

	class RModels
	{
	public:
		string modelsPath = "data/models";
		map<int, RModel> models;
		//For AITD1 - female character
		map<int, RModel> altModels;

		RModel* getModel(int idx, bool alt = false);
	};

	RModel* RModels::getModel(int idx, bool alt)
	{
		return 0;
	}

}