#pragma once
#include "../structs/life.h"

inline bool isIfInstr(LifeInstruction& instr) {
	auto t = instr.type->type;
	if (t == LifeEnum::IF_EGAL) return true;
	if (t == LifeEnum::IF_DIFFERENT) return true;
	if (t == LifeEnum::IF_SUP_EGAL) return true;
	if (t == LifeEnum::IF_SUP) return true;
	if (t == LifeEnum::IF_INF_EGAL) return true;
	if (t == LifeEnum::IF_INF) return true;
	if (t == LifeEnum::IF_IN) return true;
	if (t == LifeEnum::IF_OUT) return true;
	return false;
}

LifeNode DetectIfElse(LifeInstructionsP& insructs, int& i);
LifeNode DetectSwitch(LifeInstructionsP& insructs, int& i);

inline vector<LifeNode> lifeOptimize(LifeInstructionsP& instructs) {
	vector<LifeNode> result;
	int i = 0;
	while (i<instructs.size())
	{
		auto& ins = instructs[i];
		if (isIfInstr(*ins))
		{
			LifeNode& ln = DetectIfElse(instructs, i);
			result.push_back(ln);
		}
		else
		if (ins->type->type == LifeEnum::SWITCH)
		{
			LifeNode& ln = DetectSwitch(instructs, i);
			result.push_back(ln);
		}
		else if (ins->type->type == LifeEnum::ENDLIFE)
		{
			i++;
		}
		else
		{
			LifeNode ln;
			ln.instr = ins;
			result.push_back(ln);
			i++;
		}
	}
	return result;
}

inline LifeNode DetectIfElse(LifeInstructionsP& insructs, int &i)
{
	LifeNode ln;
	int startPos = insructs[0]->Position;
	LifeInstruction* startIns = insructs[i];
	while (isIfInstr(*insructs[i]) && startIns->Goto == insructs[i]->Goto) {
		ln.ifConditions.push_back(insructs[i]);
		i++;
	}

	int elseGoto = -1;
	LifeInstructionsP ifInstructs;
	while ((startPos + i) < startIns->Goto - 1) {
		ifInstructs.push_back(insructs[i]);
		i++;
	}
	if (insructs[i]->type->type != LifeEnum::GOTO) {
		//Without Else
		ifInstructs.push_back(insructs[i]);
		i++;
	}
	else 
	{
		//with Else
		elseGoto = insructs[i]->Goto;
		i++;
	}
	ln.ifInstructs = lifeOptimize(ifInstructs);
	if (elseGoto == -1) return ln;

	LifeInstructionsP elseInstructs;
	while ((startPos + i) < startIns->Goto - 1) {
		elseInstructs.push_back(insructs[i]);
		i++;
	}
	ln.elseInstructs = lifeOptimize(elseInstructs);
	return ln;
}

inline LifeNode DetectSwitch(LifeInstructionsP& instructs, int& i)
{
	LifeNode result;
	result.instr = instructs[i++];
	auto t = instructs[i]->type->type;
	LifeInstruction* elseGoto = 0;
	while (t == LifeEnum::CASE || t == LifeEnum::MULTI_CASE) {
		LifeCase lcase;
		lcase.caseInstr = instructs[i++];
		LifeInstructionsP caseInstructs;
		while (instructs[i]->Position < lcase.caseInstr->Goto - 1) {
			caseInstructs.push_back(instructs[i++]);
		}
		auto caseLast = instructs[i++];
		t = LifeEnum::ENDLIFE;
		if (i < instructs.size()) {
			t = instructs[i]->type->type;
		}
		if (t != LifeEnum::CASE && t != LifeEnum::MULTI_CASE) {
			//If current case last in chain - caseLast is not goto
			if (caseLast->type->type == LifeEnum::GOTO) {
				elseGoto = caseLast;
			} else {
				caseInstructs.push_back(caseLast);
			}
		}
		lcase.instructs = lifeOptimize(caseInstructs);
		result.cases.push_back(lcase);
	}
	//CASE ELSE
	if (elseGoto) {
		LifeCase lcase = { 0, {}, true };
		LifeInstructionsP caseInstructs;
		while (i < instructs.size() && instructs[i]->Position < elseGoto->Goto) {
			caseInstructs.push_back(instructs[i++]);
		}
		lcase.instructs = lifeOptimize(caseInstructs);
		result.cases.push_back(lcase);
	}

	return result;
}
