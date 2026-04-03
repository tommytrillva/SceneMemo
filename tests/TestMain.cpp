#include "TestFramework.h"

int main()
{
    int passed = 0, failed = 0;
    for (auto& test : getTests())
    {
        std::cout << "  " << test.name << "... ";
        if (test.func())
        {
            std::cout << "OK\n";
            passed++;
        }
        else
        {
            std::cout << "FAILED\n";
            failed++;
        }
    }
    std::cout << "\n" << passed << " passed, " << failed << " failed, " << (passed + failed) << " total\n";
    return failed > 0 ? 1 : 0;
}
