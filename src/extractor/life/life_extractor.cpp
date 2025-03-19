#include "../structs/int_types.h"
#include "../pak/pak.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>
#include "./life_v1.h"

using namespace std;

s16 read16(u8*& datap) {
	s16 res = READ_LE_S16(datap);
	datap += 2;
	return res;
}

LifeExpr readExpr(u8*& datap) {
	LifeExpr result;
	s16 varTypeN = read16(datap);
	if (varTypeN == -1) {
		result.constVal = read16(datap);
		return result;
	}

	auto& parse = EvalTable_v1[varTypeN];
	result.Type = parse.Type;

	for (int i = 0; i < parse.arguments.size(); i++)
	{
		LifeExpr arg;
		switch (parse.arguments[i]) {
		case lifeConst:
			arg.constVal = read16(datap);
			result.arguments.push_back(arg);
			break;
		case lifeExpr:
			arg = readExpr(datap);
			result.arguments.push_back(arg);
			break;
		}
	}

	return result;
}


LifeInstruction readInstruction(u8* &datap) {
	LifeInstruction result;
	u16 opCodeN = read16(datap);
	auto& parse = LifeTable_v1[opCodeN];	
	result.Type = parse.Type;
	//if (result.opCode == 0) throw new exception("Opcode not found");

	//if ((varTypeN & 0x8000) == 0x8000)
	//{
	//	//change actor
	//	result.Actor = read16(datap);
	//}
	//varTypeN &= 0x7FFF;

	for (int i = 0; i < parse.arguments.size(); i++)
	{
		LifeExpr arg;
		switch (parse.arguments[i]) {
			case lifeConst:
				arg.constVal = read16(datap);
				result.arguments.push_back(arg);
				break;
			case lifeGoto:
				result.Goto = read16(datap);
				break;
			case lifeExpr:
				arg = readExpr(datap);
				result.arguments.push_back(arg);
				break;
		}
	}
	return result;
}

void extractLife(string fname, string outDir)
{
	PakFile pak(fname);
	int i = 6;
	//for (int i = 0; i < pak.headers.size(); i++)
	{
		auto& data = pak.readBlock(i);
		auto datap = data.data();
		auto datap_end = datap + pak.headers[i].uncompressedSize;

		vector<LifeInstruction> life;
		while (true) {
			auto& oper = readInstruction(datap);
			life.push_back(oper);
			if (oper.Type == LifeEnum::ENDLIFE) break;
			if (datap >= datap_end) throw new exception("No ENDLIFE");
		}
	}
}
