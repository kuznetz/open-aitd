#pragma once
#include "../structs/life.h"

bool isIfInstr(LifeInstruction& instr) {
	auto t = instr.Type->Type;
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

vector<LifeNode> lifeOptimize(LifeInstructionsP& instructs) {
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
		if (ins->Type->Type == LifeEnum::SWITCH)
		{
			LifeNode& ln = DetectSwitch(instructs, i);
			result.push_back(ln);
		}
		else if (ins->Type->Type == LifeEnum::ENDLIFE)
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

LifeNode DetectIfElse(LifeInstructionsP& insructs, int &i)
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
	if (insructs[i]->Type->Type != LifeEnum::GOTO) {
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

LifeNode DetectSwitch(LifeInstructionsP& insructs, int& i)
{
	LifeNode result;
	result.instr = insructs[i++];
	auto t = insructs[i]->Type->Type;
	while (t == LifeEnum::CASE || t == LifeEnum::MULTI_CASE) {
		LifeCase lcase;
		lcase.caseInstr = insructs[i++];
		LifeInstructionsP caseInstructs;
		while (insructs[i]->Position < lcase.caseInstr->Goto - 1) {
			caseInstructs.push_back(insructs[i++]);
		}
		auto caseLast = insructs[i++];
		t = LifeEnum::ENDLIFE;
		if (i < insructs.size()) {
			t = insructs[i]->Type->Type;
		}
		if (t != LifeEnum::CASE && t != LifeEnum::MULTI_CASE) {
			//If current case last in chain - caseLast is not goto
			caseInstructs.push_back(caseLast);
		}
		lcase.instructs = lifeOptimize(caseInstructs);
		result.cases.push_back(lcase);
	}
	return result;
}
