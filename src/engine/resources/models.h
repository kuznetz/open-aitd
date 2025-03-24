#pragma once
#include <vector>
#include <map>
#include <string>
#include <raymath.h>
#include <raylib.h>
//#include <rmodels.h>

using namespace std;
namespace openAITD {

	class WModel
	{
	public:
		Model* model;
		map<int,ModelAnimation> animations;
		BoundingBox bounds;

		void load(string modelDir) {
		
		}
	};

	class WModels
	{
	public:
		string modelsPath = "data/models";
		map<int, WModel> models;
		//For AITD1 - female character
		map<int, WModel> altModels;

		WModel* getModel(int idx, bool alt = false);
	};

	WModel* WModels::getModel(int idx, bool alt = false)
	{

	}

}