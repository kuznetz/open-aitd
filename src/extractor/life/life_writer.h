#pragma once
#include "../structs/int_types.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>
#include <fstream>
#include "life_structs.h"
#include "life_optimizer.h"

void writeSpaces(ofstream& out, int level) {
	for (int l = 0; l < level; l++)
	{
		out << "  ";
	}
}

void writeLifeExpr(ofstream& out, LifeExpr& expr)
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
		writeLifeExpr(out, expr.arguments[0]);
		if (expr.arguments.size() > 1) {
			out << ", ";
			writeLifeExpr(out, expr.arguments[1]);
		}
	}
	out << ")";
}

void writeLifeInstr(ofstream& out, LifeInstruction& instr)
{
	//out << i << ": ";
	if (instr.Actor != -1) {
		out << instr.Actor << ".";
	}
	out << instr.Type->typeStr << "(";
	for (int j = 0; j < instr.arguments.size(); j++) {
		writeLifeExpr(out, instr.arguments[j]);
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

void saveLifeInstructions(ofstream& out, vector<LifeInstruction>& instructs)
{
	for (int i = 0; i < instructs.size(); i++)
	{
		auto& instr = instructs[i];
		out << i << ": ";
		writeLifeInstr(out, instr);
	}
}

void writeLifeNodes(ofstream& out, int level, vector<LifeNode>& nodes);

void writeIfHead(ofstream& out, LifeNode& ifNode)
{
	out << "if ";
	for (int i = 0; i < ifNode.ifConditions.size(); i++) {
		auto cond = ifNode.ifConditions[i];
		if (i > 0) {
			out << " and ";
		}

		out << "(";
		writeLifeExpr(out, cond->arguments[0]);
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
		writeLifeExpr(out, cond->arguments[1]);
		out << ")";
	}
	out << " then\n";
}

void writeCaseExpr(ofstream& out, LifeInstruction& switchi, LifeInstruction& instr)
{
	if (instr.Type->Type == LifeEnum::CASE) {
		writeLifeExpr(out, switchi.arguments[0]);
		out << " == ";
		out << instr.arguments[0].constVal;
	}
	else {
		for (int i = 0; i < instr.arguments.size(); i++) {
			if (i > 0) {
				out << " and ";
			}
			out << "(";
			writeLifeExpr(out, switchi.arguments[0]);
			out << " == ";
			out << instr.arguments[i].constVal;
			out << ")";
	    }
	}
}

void writeSwitch(ofstream& out, int level, LifeNode& ifNode)
{
	for (int i = 0; i < ifNode.cases.size(); i++) {
		writeSpaces(out, level);
		if (i == 0) {
			out << "if ";
		} else {
			out << "elseif ";
		}
		writeCaseExpr(out, *ifNode.instr, *ifNode.cases[i].caseInstr);
		out << " then\n";
		writeLifeNodes(out, level+1, ifNode.cases[i].instructs);
	}
	writeSpaces(out, level);
	out << "end\n";
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
			writeSpaces(out, level);
			writeIfHead(out, node);
			writeLifeNodes(out, level + 1, node.ifInstructs);
			if (node.elseInstructs.size()) {
				writeSpaces(out, level);
				out << "else\n";
				writeLifeNodes(out, level + 1, node.elseInstructs);
			}
			writeSpaces(out, level);
			out << "end\n";
		}
		else if (node.cases.size())
		{
			writeSwitch(out, level, node);
		}
		else
		{
			writeSpaces(out, level);
			writeLifeInstr(out, *node.instr);
		}
	}
	//out << "end";
}