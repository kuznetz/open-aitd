#include "../structs/int_types.h"
#include "../pak/pak.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>
#include "./life_v1.h"
#include <fstream>

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


LifeInstruction readInstruction(lifeBuffer &buf) {
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

vector<LifeInstruction> loadLife(u8* data, int size)
{
	lifeBuffer buf = {
		data,
		data + size
	};
	vector<LifeInstruction> life;
	while (true) {
		auto& oper = readInstruction(buf);
		oper.Position = buf.data - data;
		life.push_back(oper);
		if (oper.Type->Type == LifeEnum::ENDLIFE) break;
	}

	for (int i = 0; i < life.size(); i++) {
		if (life[i].Goto == -1) continue;
		auto gotoPos = life[i].Position + (life[i].Goto * 2);
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

	return life;
}

void saveExpr(ofstream& out, LifeExpr& expr)
{
	if (!expr.Type) {
		out << to_string(expr.constVal);
		return;
	}
	if (expr.Actor != -1) {
		out << expr.Actor << ".";
	}
	out << expr.Type->typeStr;
	if (expr.arguments.size() > 0) {
		out << "(";
		saveExpr(out, expr.arguments[0]);
		if (expr.arguments.size() > 1) {
			out << ", ";
			saveExpr(out, expr.arguments[1]);
		}
		out << ")";
	}
}

void saveLife(ofstream& out, vector<LifeInstruction>& instructs)
{
	for (int i = 0; i < instructs.size(); i++)
	{
		auto& instr = instructs[i];
		out << i << ": ";
		if (instr.Actor != -1) {
			out << instr.Actor << ".";
		}
		out << instr.Type->typeStr << " (";
		for (int j = 0; j < instr.arguments.size(); j++) {
			saveExpr(out, instr.arguments[j]);
			if (j < instr.arguments.size()-1) {
				out << ", ";
			}
		}
		out << ")";
		if (instr.Goto != -1) {
			out << " " << to_string(instr.Goto);
		}
		out << "\n";
	}
}

void extractLife(string fname, string outFile)
{
	PakFile pak(fname);
	ofstream out(outFile, ios::trunc | ios::out);
	//int i = 7;
	for (int i = 0; i < pak.headers.size(); i++)
	{
		out << "LIFE " << i << "\n";
		auto& data = pak.readBlock(i);
		auto& life = loadLife(data.data(), pak.headers[i].uncompressedSize);
		saveLife(out, life);
		out << "\n";
	}
	out.close();
}