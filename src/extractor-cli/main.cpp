//#include "tests/tests.h"
#include "../extractor/extractor.hpp"

int main(void)
{
    //linkAnimations();
    AITDExtractor::AITDExtractor extractor;
    extractor.extractAllData(false);
    return 0;
}
