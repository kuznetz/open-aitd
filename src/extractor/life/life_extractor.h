#pragma once
#include "../structs/int_types.h"
#include "../pak/pak.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>
#include <fstream>
#include "life_v1.h"
#include "life_optimizer.h"
#include "life_writer.h"

using namespace std;

struct lifeBuffer {
	u8* data;
	u8* dataEnd;
};

inline s16 read16(lifeBuffer& buf) {
	if (buf.data >= buf.dataEnd) throw new exception("read16 over buffer size");
    s16 res = READ_LE_S16(buf.data);
	buf.data += 2;
	return res;
}

inline LifeExpr readExpr(lifeBuffer& buf) {
	LifeExpr result;
	s16 varTypeN = read16(buf);
	if (varTypeN == -1) {
		result.constVal = read16(buf);
		return result;
	}

	if ((varTypeN & 0x8000) == 0x8000)
	{
		//change actor
		result.Actor = read16(buf);
	}
	varTypeN &= 0x7FFF;

	auto& parse = ExprTable_v1[varTypeN];
	result.Type = &parse;

	//if (result.Type == LifeEnum::READ) {}

	for (int i = 0; i < parse.arguments.size(); i++)
	{
		LifeExpr arg;
		switch (parse.arguments[i]) {
		case lifeConst:
			arg.constVal = read16(buf);
			result.arguments.push_back(arg);
			break;
		case lifeExpr:
			arg = readExpr(buf);
			result.arguments.push_back(arg);
			break;
		}
	}

	return result;
}


inline LifeInstruction readInstruction(lifeBuffer &buf) {
	LifeInstruction result;
	u16 opCodeN = read16(buf);

	if ((opCodeN & 0x8000) == 0x8000)
	{
		//change actor
		result.Actor = read16(buf);
	}
	opCodeN &= 0x7FFF;

	auto& parse = LifeTable_v1[opCodeN];
	result.Type = &parse;

	if (parse.Type == LifeEnum::MULTI_CASE)
	{
		int numCases = read16(buf);
		for (int n = 0; n < numCases; n++)
		{
			LifeExpr arg;
			arg.constVal = read16(buf);
			result.arguments.push_back(arg);
		}
		result.Goto = read16(buf);
	}
	else
	{
		for (int i = 0; i < parse.arguments.size(); i++)
		{
			LifeExpr arg;
			switch (parse.arguments[i]) {
			case lifeConst:
				arg.constVal = read16(buf);
				result.arguments.push_back(arg);
				break;
			case lifeGoto:
				result.Goto = read16(buf);
				break;
			case lifeExpr:
				arg = readExpr(buf);
				result.arguments.push_back(arg);
				break;
			}
		}
	}

	return result;
}

inline vector<LifeInstruction> loadLife(u8* data, int size)
{
	lifeBuffer buf = {
		data,
		data + size
	};
	vector<LifeInstruction> life;
	while (true) {
		int pos = buf.data - data;
		auto& oper = readInstruction(buf);
		oper.Size = (buf.data - data) - pos;
		oper.Position = pos;
		life.push_back(oper);
		if (oper.Type->Type == LifeEnum::ENDLIFE) break;
	}

	//Fix Goto
	for (int i = 0; i < life.size(); i++) {
		if (life[i].Goto == -1) continue;
		auto gotoPos = life[i].Position + life[i].Size + (life[i].Goto * 2);
		bool changed = false;
		for (int j = 0; j < life.size(); j++) {
			if (gotoPos == life[j].Position) {
				life[i].Goto = j;
				changed = true;
				break;
			}
		}
		if (!changed) 
		{
			throw new exception("Cant calc goto");
		}
	}

	//Fix Position
	for (int i = 0; i < life.size(); i++) {
		life[i].Position = i;
	}

	return life;
}

inline void extractLife(string fname, string outFile)
{
	PakFile pak(fname);
	vector<vector<LifeInstruction>> lifes;
	vector<vector<LifeNode>> lifesNodes;
	//int i = 514;
	for (int i = 0; i < pak.headers.size(); i++)
	{
		auto& data = pak.readBlock(i);
		lifes.push_back(loadLife(data.data(), pak.headers[i].uncompressedSize));
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
	for (int j = 0; j < lifesNodes.size(); j++)
	{
		out << "function life_" << j << "(obj)\n";
		writeLifeNodes(out, 1, lifesNodes[j]);
		out << "end\n\n";
	}
	out.close();
}