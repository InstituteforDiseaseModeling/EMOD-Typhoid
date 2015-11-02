#include "CurrentTest.h"
#include <cstddef>

namespace UnitTest {

    TestResults*& CurrentTest::Results()
    {
        static TestResults* testResults = nullptr;
        return testResults;
    }

    const TestDetails*& CurrentTest::Details()
    {
        static const TestDetails* testDetails = nullptr;
        return testDetails;
    }
}
