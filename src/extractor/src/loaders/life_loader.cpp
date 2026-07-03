#include "./life_v1.h"
#include "./loaders.h"

namespace AITDExtractor {
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

		auto& parse = LifeExprParams_V1[varTypeN];
		result.type = &parse;

		//if (result.type == LifeEnum::READ) {}

		for (int i = 0; i < parse.arguments.size(); i++)
		{
			LifeExpr arg;
			switch (parse.arguments[i].type) {
			case lifeConst:
				arg.constVal = read16(buf);
				arg.constType = parse.arguments[i].varType;
				result.arguments.push_back(arg);
				break;
			case lifeExpr:
				arg = readExpr(buf);
				arg.constType = parse.arguments[i].varType;
				result.arguments.push_back(arg);
				break;
			}
		}

		return result;
	}  

	inline LifeInstruction readInstruction(lifeBuffer &buf, bool floppy = false) {
		LifeInstruction result;
		u16 opCodeN = read16(buf);

		if ((opCodeN & 0x8000) == 0x8000)
		{
			//change actor
			result.Actor = read16(buf);
		}
		opCodeN &= 0x7FFF;

		auto* parse = &LifeParams_V1[opCodeN];
		if (floppy && parse->type == LifeEnum::READ) {
			parse = &LifeParams_Floppy_V1;
		}


		result.type = parse;
		if (parse->type == LifeEnum::MULTI_CASE)
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
			for (int i = 0; i < parse->arguments.size(); i++)
			{
				LifeExpr arg;
				switch (parse->arguments[i].type) {
				case lifeConst:
					arg.constVal = read16(buf);
					arg.constType = parse->arguments[i].varType;
					result.arguments.push_back(arg);
					break;
				case lifeGoto:
					result.Goto = read16(buf);
					break;
				case lifeExpr:
					arg = readExpr(buf);
  				arg.constType = parse->arguments[i].varType;
					result.arguments.push_back(arg);
					break;
				}
			}
		}

		return result;
	}  

  vector<LifeInstruction> ResourceLoader::loadLife(vector<u8>& vdata, bool floppy)
  {
    u8* data = vdata.data();
    int size = vdata.size();
    lifeBuffer buf = {
      data,
      data + size
    };
    vector<LifeInstruction> life;
    while (true) {
      int pos = buf.data - data;
      auto& oper = readInstruction(buf, floppy);
      oper.Size = (buf.data - data) - pos;
      oper.Position = pos;
      life.push_back(oper);
      if (oper.type->type == LifeEnum::ENDLIFE) break;
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

}