#include "tests/tests.h"
#include "extractor/main_extractor.h"

int main(void)
{
    extractAllData();

    //CameraTest::runTest();
    //MaskTest::runTest();
    //ModelTest::runTest();
    PerspectiveTest::runTest();

    return 0;
}
