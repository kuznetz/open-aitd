#pragma once
#include "../structs/int_types.h"
#include "../pak/pak.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include "life_optimizer.h"
#include "life_writer.hpp"

namespace AITDExtractor {
  using namespace std;
	using namespace openAITD;

	inline void extractLife(string fname, string outFile, int varCount, NameDecoders& nameDecs, int objectCount, int modelCount, bool floppy = false)
	{
		PakFile pak(fname);
		vector<vector<LifeInstruction>> lifes;
		vector<vector<LifeNode>> lifesNodes;
		//int i = 514;
		for (int i = 0; i < pak.headers.size(); i++)
		{
			auto& data = pak.readBlock(i);
			lifes.push_back(loadLife(data.data(), pak.headers[i].uncompressedSize, floppy));
			auto& life = lifes.back();

			LifeInstructionsP lifep;
			auto lifeData = life.data();
			for (int j = 0; j < life.size(); j++) {
				lifep.push_back(lifeData + j);
			}
				auto& nodes = lifeOptimize(lifep);
			lifesNodes.push_back(nodes);
		}

		//ofstream out(outFile, ios::trunc | ios::out);
		//for (int j = 0; j < lifes.size(); j++)
		//{
		//	out << "LIFE " << j << "\n";
		//	saveLifeInstructions(out, lifes[j]);
		//	out << "\n";
		//}
		//out.close();

		ofstream out(outFile, ios::trunc | ios::out);
		LifeLUAWriter writer(out, nameDecs);
		writer.writeConsts(varCount, objectCount, modelCount);		
		for (int j = 0; j < lifesNodes.size(); j++)
		{
			out << "-- " << nameDecs.life.getName(j, true) << "\n";
			out << "function life_" << j << "(obj)\n";
			writer.writeLifeNodes(1, lifesNodes[j]);
			out << "end\n\n";
		}
		out.close();
	}

	inline string to_hex(int i) {
		std::stringstream strs;
		strs << "0x"
			<< std::setfill('0') << std::setw(4)
			<< std::hex << i;
		return strs.str();
	}

	inline void dumpInstructions(string outFile) {
		ofstream out(outFile, ios::trunc | ios::out);
		out << "EXPR:\n";
		for (int j = 0; j < LifeExprParams_V1.size(); j++)
		{
			auto& e = LifeExprParams_V1[j];
			out << e.typeStr << " " << to_string(j) << " " << to_hex(j) << "\n";
		}

		out << "\nLIFE:\n";
		for (int j = 0; j < LifeParams_V1.size(); j++)
		{
			auto& e = LifeParams_V1[j];
			out << e.typeStr << " " << to_string(j) << " " << to_hex(j) << "\n";
		}
		out.close();

	}

}