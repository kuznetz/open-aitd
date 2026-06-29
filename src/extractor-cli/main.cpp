//#include "tests/tests.h"
#include "../extractor/include/extractor.h"

int main(void)
{
    //linkAnimations();
    auto extractor = AITDExtractor::createAITDExtractor();
    extractor->extractAllData(false);
    return 0;
}
