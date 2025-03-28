//#include "tests/tests.h"
//#include "../extractor/extractor.h"
//#include "../extractor/anim_linker.h"

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
    
    ModelTest::runTest();    
    //PerspectiveTest::runTest();

    return 0;
}
