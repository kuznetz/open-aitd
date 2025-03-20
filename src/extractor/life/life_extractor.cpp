#include "../structs/int_types.h"
#include "../pak/pak.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>
#include <fstream>
#include "./life_v1.h"
#include "./life_optimizer.h"

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

void saveExprTxt(ofstream& out, LifeExpr& expr)
{
	if (!expr.Type) {
		out << to_string(expr.constVal);
		return;
	}
	if (expr.Actor != -1) {
		out << expr.Actor << ".";
	}
	out << expr.Type->typeStr;
	out << "(";
	if (expr.arguments.size() > 0) {
		saveExprTxt(out, expr.arguments[0]);
		if (expr.arguments.size() > 1) {
			out << ", ";
			saveExprTxt(out, expr.arguments[1]);
		}
	}
	out << ")";
}

void saveLifeTxt(ofstream& out, vector<LifeInstruction>& instructs)
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
			saveExprTxt(out, instr.arguments[j]);
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

void spaces(ofstream& out, int level) {
	for (int l = 0; l < level; l++)
	{
		out << "  ";
	}
}

void writeLifeInstr(ofstream& out, LifeInstruction& instr)
{
	//out << i << ": ";
	if (instr.Actor != -1) {
		out << instr.Actor << ".";
	}
	out << instr.Type->typeStr << "(";
	for (int j = 0; j < instr.arguments.size(); j++) {
		saveExprTxt(out, instr.arguments[j]);
		if (j < instr.arguments.size() - 1) {
			out << ", ";
		}
	}
	out << ")";
	if (instr.Goto != -1) {
		out << " " << to_string(instr.Goto);
	}
	out << "\n";
}

void writeIfHead(ofstream& out, LifeNode& ifNode)
{
	out << "if ";
	for (int i = 0; i < ifNode.ifConditions.size(); i++) {
		auto cond = ifNode.ifConditions[i];
		if (i > 0) {
			out << " and ";
		}

		out << "(";
		saveExprTxt(out, cond->arguments[0]);
		switch (cond->Type->Type) {
		case LifeEnum::IF_EGAL:
			out << " == ";
			break;
		case LifeEnum::IF_DIFFERENT:
			out << " == ";
			break;
		default:
			out << " ??? ";
		}
		saveExprTxt(out, cond->arguments[1]);
		out << ")";
	}
	out << "\n";
}


void writeLifeNodes(ofstream& out, int level, vector<LifeNode>& nodes)
{
	//out << "function life_" << lifeIdx << "(o)\n";
	//vector<int> gotos;
	//for (int i = 0; i < nodes.size(); i++)
	//{
	//	if (instructs[i].Goto == -1) continue;
	//	gotos.push_back(instructs[i].Goto);
	//}

	for (int i = 0; i < nodes.size(); i++)
	{
		auto& node = nodes[i];
		if (node.ifConditions.size())
		{
			spaces(out, level);
			writeIfHead(out, node);
			writeLifeNodes(out, level + 1, node.ifInstructs);
			if (node.elseInstructs.size()) {
				spaces(out, level);
				out << "else\n";
				writeLifeNodes(out, level + 1, node.elseInstructs);
			}
			spaces(out, level);
			out << "end\n";
		}
		else
		{
			spaces(out, level);
			writeLifeInstr(out, *node.instr);
		}
	}
	//out << "end";
}


void extractLife(string fname, string outFile)
{
	PakFile pak(fname);
	vector<vector<LifeInstruction>> lifes;
	vector<vector<LifeNode>> lifesNodes;
	int i = 0;
	//for (int i = 0; i < pak.headers.size(); i++)
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
	//	saveLifeTxt(out, lifes[j]);
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