#include "pch.h"
#include "Test.h"

using namespace mu;

static std::vector<TestEntry> g_tests;

void RegisterTestEntryImpl(const TestEntry& v)
{
    g_tests.push_back(v);
}

static void ExecuteTest(const TestEntry& v)
{
    printf("%s begin\n", v.name.c_str());
    auto begin = Now();
    v.body();
    auto end = Now();
    printf("%s end (%fms)\n\n", v.name.c_str(), (end-begin)/1000000.0f);
}

int main(int argc, char *argv[])
{
    if (argc == 1) {
        for (auto& entry : g_tests) {
            ExecuteTest(entry);
        }
    }
    else {
        for (auto& entry : g_tests) {
            for (int ai = 1; ai < argc; ++ai) {
                if (entry.name == argv[ai]) {
                    ExecuteTest(entry);
                }
            }
        }
    }

    printf("Press any key to exit\n");
    char dummy;
    scanf("%c", &dummy);
}
