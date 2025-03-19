#include "../structs/int_types.h"
#include "../pak/pak.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>
#include "./life_v1.h"

using namespace std;

struct lifeBuffer {
	u8* data;
	u8* dataEnd;
};

s16 read16(lifeBuffer& buf) {
	if (buf.data >= buf.dataEnd) throw new exception("read16 over buffer size");
    s16 res = READ_LE_S16(buf.data);
	buf.data += 2;
	return res;
}

LifeExpr readExpr(lifeBuffer& buf) {
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
	result.Type = parse.Type;

	if (!parse.checked) {
		printf("Expression Not checked");
	}

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


LifeInstruction readInstruction(lifeBuffer &buf) {
	LifeInstruction result;
	u16 opCodeN = read16(buf);
	auto& parse = LifeTable_v1[opCodeN];	
	result.Type = parse.Type;
	//if (result.opCode == 0) throw new exception("Opcode not found");

	if (!parse.checked) {
		printf("Instruction Not checked");
	}

	if (result.Type == LifeEnum::MULTI_CASE)
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

void extractLife(string fname, string outDir)
{
	PakFile pak(fname);
	int i = 7;
	//for (int i = 0; i < pak.headers.size(); i++)
	{
		auto& data = pak.readBlock(i);
		lifeBuffer buf = {
			data.data(),
			data.data() + pak.headers[i].uncompressedSize
		};
		vector<LifeInstruction> life;
		while (true) {
			auto& oper = readInstruction(buf);
			life.push_back(oper);
			if (oper.Type == LifeEnum::ENDLIFE) break;
		}
	}
}
