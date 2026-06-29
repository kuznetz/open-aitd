#pragma once

#include "../structs/int_types.h"
#include <cstdio>
#include <string>
#include <fstream>
#include <iomanip>
#include <vector>
#include <stdexcept>
#include "../../names-decode/name_decoders.hpp"
#include "../structs/life.h"


namespace AITDExtractor {
	using namespace openAITD;

	/**
	 * @brief Utility class for writing LUA code from Life Scripts
	 */
	class LifeLUAWriter {
	public:
			/**
			 * @brief Construct a writer that outputs to the given stream.
			 */
			explicit LifeLUAWriter(std::ofstream& outStream, NameDecoders& namesDecoders) : out(outStream) {
					this->namesDecoders = &namesDecoders;
			}

			/**
			 * @brief Write a Life expression.
			 * @param expr Expression to write.
			 */
			void writeLifeExpr(const LifeExpr& expr);

			/**
			 * @brief Write a single Life instruction.
			 * @param instr Instruction to write.
			 */
			void writeLifeInstr(const LifeInstruction& instr);

			/**
			 * @brief Write a whole list of instructions with line numbers.
			 * @param instructs Vector of instructions.
			 */
			void saveLifeInstructions(const std::vector<LifeInstruction>& instructs);

			/**
			 * @brief Write a list of Life nodes (blocks) recursively.
			 * @param level Indentation level (number of spaces).
			 * @param nodes Nodes to write.
			 */
			void writeLifeNodes(int level, const std::vector<LifeNode>& nodes);

			/**
			 * @brief Write constant definitions.
			 */
			void writeConsts(int varCount, int objectCount, int modelCount);

	private:
			std::ofstream& out;  ///< Output stream reference.
			NameDecoders* namesDecoders;  ///< Name decoder instance.

			/**
			 * @brief Write indentation spaces.
			 * @param level Number of indent levels (2 spaces each).
			 */
			void writeSpaces(int level);

			/**
			 * @brief Write the header of an 'if' block.
			 * @param ifNode Node containing the condition list.
			 */
			void writeIfHead(const LifeNode& ifNode);

			/**
			 * @brief Write a CASE or MULTI_CASE expression for a switch.
			 * @param varsw Name of the switch variable.
			 * @param instr Instruction that holds the case values.
			 */
			void writeCaseExpr(const std::string& varsw, const LifeInstruction& instr);

			/**
			 * @brief Write a complete switch (CASE/MULTI_CASE) block.
			 * @param level Indentation level.
			 * @param ifNode Node representing the switch.
			 */
			void writeSwitch(int level, const LifeNode& ifNode);
	};

	// ----------------------------------------------------------------------------

	inline void LifeLUAWriter::writeSpaces(int level) {
			for (int l = 0; l < level; ++l) {
					out << "  ";
			}
	}

	inline void LifeLUAWriter::writeLifeExpr(const LifeExpr& expr) {
			if (!expr.type) {
				  if (expr.constVal < 0) {
						out << std::to_string(expr.constVal);
					} else if (expr.constType == varObject) {
						out << "GObj." << namesDecoders->obj.getName(expr.constVal);
					} else if (expr.constType == varModel) {
						out << "Model." <<  namesDecoders->model.getName(expr.constVal);
					} else if (expr.constType == varLifeScript) {
						out << "Life." <<  namesDecoders->life.getName(expr.constVal);
					} else {
						out << std::to_string(expr.constVal);
					}
					return;
			}
			if (expr.type->type == EvalEnum::GET) {
					out << "var_" << expr.arguments[0].constVal;
					return;
			}

			out << expr.type->typeStr;
			out << "(";

			if (expr.type->needActor && expr.Actor != -1) {
					out << "GObj." << namesDecoders->obj.getName(expr.Actor);
			} else if (expr.type->needActor && expr.Actor == -1) {
					out << "obj";
			} else if (!expr.type->needActor && expr.Actor != -1) {
					throw std::runtime_error("Has actor, but needActor = false");
			}

			if (!expr.arguments.empty()) {
					if (expr.type->needActor) {
							out << ", ";
					}
					writeLifeExpr(expr.arguments[0]);
					if (expr.arguments.size() > 1) {
							out << ", ";
							writeLifeExpr(expr.arguments[1]);
					}
			}
			out << ")";
	}

	inline void LifeLUAWriter::writeLifeInstr(const LifeInstruction& instr) {
			// Short‑hand for common arithmetic operations
			if (instr.type->type == LifeEnum::INC) {
					std::string v = "var_" + std::to_string(instr.arguments[0].constVal);
					out << v << " = " << v << " + 1\n";
					return;
			} else if (instr.type->type == LifeEnum::DEC) {
					std::string v = "var_" + std::to_string(instr.arguments[0].constVal);
					out << v << " = " << v << " - 1\n";
					return;
			} else if (instr.type->type == LifeEnum::ADD) {
					std::string v = "var_" + std::to_string(instr.arguments[0].constVal);
					out << v << " = " << v << " + ";
					writeLifeExpr(instr.arguments[1]);
					out << "\n";
					return;
			} else if (instr.type->type == LifeEnum::SUB) {
					std::string v = "var_" + std::to_string(instr.arguments[0].constVal);
					out << v << " = " << v << " - ";
					writeLifeExpr(instr.arguments[1]);
					out << "\n";
					return;
			} else if (instr.type->type == LifeEnum::SET) {
					std::string v = "var_" + std::to_string(instr.arguments[0].constVal);
					out << v << " = ";
					writeLifeExpr(instr.arguments[1]);
					out << "\n";
					return;
			}

			// Generic instruction output
			out << instr.type->typeStr << "(";

			if (instr.type->needActor && instr.Actor != -1) {
					out << "GObj." << namesDecoders->obj.getName(instr.Actor);
			} else if (instr.type->needActor && instr.Actor == -1) {
					out << "obj";
			} else if (!instr.type->needActor && instr.Actor != -1) {
					throw std::runtime_error("Has actor, but needActor = false");
			}

			if (!instr.arguments.empty() && instr.type->needActor) {
					out << ", ";
			}
			for (std::size_t j = 0; j < instr.arguments.size(); ++j) {
					writeLifeExpr(instr.arguments[j]);
					if (j < instr.arguments.size() - 1) {
							out << ", ";
					}
			}
			out << ")";
			if (instr.Goto != -1) {
					out << " " << std::to_string(instr.Goto);
			}
			out << "\n";
	}

	inline void LifeLUAWriter::saveLifeInstructions(const std::vector<LifeInstruction>& instructs) {
			for (std::size_t i = 0; i < instructs.size(); ++i) {
					out << i << ": ";
					writeLifeInstr(instructs[i]);
			}
	}

	inline void LifeLUAWriter::writeLifeNodes(int level, const std::vector<LifeNode>& nodes) {
			for (const auto& node : nodes) {
					if (!node.ifConditions.empty()) {
							writeSpaces(level);
							writeIfHead(node);
							writeLifeNodes(level + 1, node.ifInstructs);
							if (!node.elseInstructs.empty()) {
									writeSpaces(level);
									out << "else\n";
									writeLifeNodes(level + 1, node.elseInstructs);
							}
							writeSpaces(level);
							out << "end\n";
					} else if (!node.cases.empty()) {
							writeSwitch(level, node);
					} else if (node.instr->type->type == LifeEnum::RETURN) {
							writeSpaces(level);
							out << "do return end\n";
					} else {
							writeSpaces(level);
							writeLifeInstr(*node.instr);
					}
			}
	}

	inline void LifeLUAWriter::writeConsts(int scriptCount, int objectCount, int modelCount) {
			out << "-- Game Objects table\n";
			out << "GObj = {}\n";
			for (int i = 0; i < objectCount; ++i) {
					out << "GObj." << namesDecoders->obj.getName(i) << " = " << i << "\n";
			}

			out << "\n-- Models table\n";
			out << "Model = {}\n";
			for (int i = 0; i < modelCount; ++i) {
					out << "Model." << namesDecoders->model.getName(i) << " = " << i << "\n";
			}

			out << "\n-- Life Scripts table\n";
			out << "Life = {}\n";
			for (int i = 0; i < scriptCount; ++i) {
					out << "Life." << namesDecoders->life.getName(i) << " = " << i << "\n";
			}
	}

	inline void LifeLUAWriter::writeIfHead(const LifeNode& ifNode) {
			out << "if ";
			for (std::size_t i = 0; i < ifNode.ifConditions.size(); ++i) {
					auto cond = ifNode.ifConditions[i];
					if (i > 0) {
							out << " and ";
					}
					out << "(";

					if (!!cond->arguments[0].type && !cond->arguments[1].type) {
						cond->arguments[1].constType = cond->arguments[0].type->returnType;
					}
					if (!!cond->arguments[1].type && !cond->arguments[0].type) {
						cond->arguments[0].constType = cond->arguments[1].type->returnType;
					}

					writeLifeExpr(cond->arguments[0]);
					switch (cond->type->type) {
							case LifeEnum::IF_EGAL:       out << " == "; break;
							case LifeEnum::IF_DIFFERENT:  out << " ~= "; break;
							case LifeEnum::IF_SUP_EGAL:   out << " >= "; break;
							case LifeEnum::IF_SUP:        out << " > ";  break;
							case LifeEnum::IF_INF_EGAL:   out << " <= "; break;
							case LifeEnum::IF_INF:        out << " < ";  break;
							case LifeEnum::IF_IN:         out << " IF_IN "; break;
							case LifeEnum::IF_OUT:        out << " IF_OUT "; break;
							default:                      out << " ??? ";
					}
  				writeLifeExpr(cond->arguments[1]);
					out << ")";
			}
			out << " then\n";
	}

	inline void LifeLUAWriter::writeCaseExpr(const std::string& varsw, const LifeInstruction& instr) {
			if (instr.type->type == LifeEnum::CASE) {
					out << varsw << " == ";
					out << instr.arguments[0].constVal;
			} else if (instr.type->type == LifeEnum::MULTI_CASE) {
					for (std::size_t i = 0; i < instr.arguments.size(); ++i) {
							if (i > 0) {
									out << " or ";
							}
							out << "(";
							out << varsw << " == ";
							out << instr.arguments[i].constVal;
							out << ")";
					}
			}
	}

	inline void LifeLUAWriter::writeSwitch(int level, const LifeNode& ifNode) {
			writeSpaces(level);
			std::string varsw = "sw_" + std::to_string(level);
			out << varsw << " = ";
			writeLifeExpr(ifNode.instr->arguments[0]);
			out << "\n";

			for (std::size_t i = 0; i < ifNode.cases.size(); ++i) {
					writeSpaces(level);
					if (i == 0) {
							out << "if ";
							writeCaseExpr(varsw, *ifNode.cases[i].caseInstr);
							out << " then\n";
					} else if (!ifNode.cases[i].isElse) {
							out << "elseif ";
							writeCaseExpr(varsw, *ifNode.cases[i].caseInstr);
							out << " then\n";
					} else {
							out << "else\n";
					}
					writeLifeNodes(level + 1, ifNode.cases[i].instructs);
			}
			writeSpaces(level);
			out << "end\n";
	}

}