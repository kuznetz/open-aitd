//#include "tests/tests.h"
//#include "../extractor/extractor.h"
//#include "../extractor/anim_linker.h"
//#include "posrel_test.h"
#include "animation_test.h"

namespace PerspectiveTest {
    void runTest();
}

namespace ModelTest {
    void runTest();
}

int main(void)
{
    //CameraTest::runTest();
    //MaskTest::runTest();    
    //ModelTest::runTest();    
    //PerspectiveTest::runTest();
    //PosRelTest::runTest();
    AnimationTest::runTest();
    return 0;
}
