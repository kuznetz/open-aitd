#pragma once
#include "../structs/int_types.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>
#include <fstream>
#include "../structs/life.h"

inline void writeSpaces(ofstream& out, int level) {
	for (int l = 0; l < level; l++)
	{
		out << "  ";
	}
}

inline void writeLifeExpr(ofstream& out, LifeExpr& expr)
{
	if (!expr.type) {
		out << to_string(expr.constVal);
		return;
	}
	if (expr.type->type == EvalEnum::GET ) {
		out << "var_" << expr.arguments[0].constVal;
		return;
	}
	out << expr.type->typeStr;
	out << "(";
	if (expr.type->needActor && expr.Actor != -1) {
		out << expr.Actor;
	}
	else if (expr.type->needActor && expr.Actor == -1) {
		out << "obj";
	}
	else if (!expr.type->needActor && expr.Actor != -1) {
		throw new exception("Has actor, but needActor = false");
	}
	if (expr.arguments.size() > 0) {
		if (expr.type->needActor) {
			out << ", ";
		}
		writeLifeExpr(out, expr.arguments[0]);
		if (expr.arguments.size() > 1) {
			out << ", ";
			writeLifeExpr(out, expr.arguments[1]);
		}
	}
	out << ")";
}

inline void writeLifeInstr(ofstream& out, LifeInstruction& instr)
{
	//out << i << ": ";

	if (instr.type->type == LifeEnum::INC) {
		string v = "var_"+to_string(instr.arguments[0].constVal);
		out << v << " = " << v << " + 1\n";
		return;
	} else if (instr.type->type == LifeEnum::DEC) {
		string v = "var_" + to_string(instr.arguments[0].constVal);
		out << v << " = " << v << " - 1\n";
		return;
	} else if (instr.type->type == LifeEnum::ADD) {
		string v = "var_" + to_string(instr.arguments[0].constVal);
		out << v << " = " << v << " + ";
		writeLifeExpr(out, instr.arguments[1]);
		out << "\n";
		return;
	} else if (instr.type->type == LifeEnum::SUB) {
		string v = "var_" + to_string(instr.arguments[0].constVal);
		out << v << " = " << v << " - ";
		writeLifeExpr(out, instr.arguments[1]);
		out << "\n";
		return;
	} else if (instr.type->type == LifeEnum::SET) {
		string v = "var_" + to_string(instr.arguments[0].constVal);
		out << v << " = ";
		writeLifeExpr(out, instr.arguments[1]);
		out << "\n";
		return;
	}

	out << instr.type->typeStr << "(";
	if (instr.type->needActor && instr.Actor != -1) {
		out << instr.Actor;
	}
	else if(instr.type->needActor && instr.Actor == -1) {
		out << "obj";
	}
	else if (!instr.type->needActor && instr.Actor != -1) {
		throw new exception("Has actor, but needActor = false");
	}
	if (instr.arguments.size() > 0 && instr.type->needActor) {
		out << ", ";
	}
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

inline void saveLifeInstructions(ofstream& out, vector<LifeInstruction>& instructs)
{
	for (int i = 0; i < instructs.size(); i++)
	{
		auto& instr = instructs[i];
		out << i << ": ";
		writeLifeInstr(out, instr);
	}
}

inline void writeLifeNodes(ofstream& out, int level, vector<LifeNode>& nodes);

inline void writeIfHead(ofstream& out, LifeNode& ifNode)
{
	out << "if ";
	for (int i = 0; i < ifNode.ifConditions.size(); i++) {
		auto cond = ifNode.ifConditions[i];
		if (i > 0) {
			out << " and ";
		}
		out << "(";
		writeLifeExpr(out, cond->arguments[0]);
		switch (cond->type->type) {
		case LifeEnum::IF_EGAL:
			out << " == ";
			break;
		case LifeEnum::IF_DIFFERENT:
			out << " ~= ";
			break;
		case LifeEnum::IF_SUP_EGAL:
			out << " >= ";
			break;
		case LifeEnum::IF_SUP:
			out << " > ";
			break;
		case LifeEnum::IF_INF_EGAL:
			out << " <= ";
			break;
		case LifeEnum::IF_INF:
			out << " < ";
			break;
		case LifeEnum::IF_IN:
			out << " IF_IN ";
			break;
		case LifeEnum::IF_OUT:
			out << " IF_OUT ";
			break;
		default:
			out << " ??? ";
		}
		writeLifeExpr(out, cond->arguments[1]);
		out << ")";
	}
	out << " then\n";
}

inline void writeCaseExpr(ofstream& out, string varsw, LifeInstruction& instr)
{
    if (instr.type->type == LifeEnum::CASE) {
		out << varsw << " == ";
		out << instr.arguments[0].constVal;
	}
	else if(instr.type->type == LifeEnum::MULTI_CASE) {
		for (int i = 0; i < instr.arguments.size(); i++) {
			if (i > 0) {
				out << " and ";
			}
			out << "(";
			out << varsw << " == ";
			out << instr.arguments[i].constVal;
			out << ")";
	    }
	}
}

inline void writeSwitch(ofstream& out, int level, LifeNode& ifNode)
{
	writeSpaces(out, level);
	string varsw = string("sw_") + to_string(level);
	out << varsw << " = ";
	writeLifeExpr(out, ifNode.instr->arguments[0]);
	out << "\n";
	for (int i = 0; i < ifNode.cases.size(); i++) {
		writeSpaces(out, level);
		if (i == 0) {
			out << "if ";
			writeCaseExpr(out, varsw, *ifNode.cases[i].caseInstr);
			out << " then\n";
		} else if (!ifNode.cases[i].isElse) {
			out << "elseif ";
			writeCaseExpr(out, varsw, *ifNode.cases[i].caseInstr);
			out << " then\n";
		}
		else {
			out << "else\n";
		}
		writeLifeNodes(out, level+1, ifNode.cases[i].instructs);
	}
	writeSpaces(out, level);
	out << "end\n";
}

inline void writeLifeNodes(ofstream& out, int level, vector<LifeNode>& nodes)
{
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
		else if (node.instr->type->type == LifeEnum::RETURN)
		{
			writeSpaces(out, level);
			out << "do return end\n";
		}
		else
		{
			writeSpaces(out, level);
			writeLifeInstr(out, *node.instr);
		}
	}
}
