#include "pch.h"
#include "Test.h"
#include "MeshGenerator.h"

using namespace mu;

void PrintImpl(const char *format, ...)
{
    va_list args;
    va_start(args, format);
#ifdef _WIN32
    const int MaxBuf = 4096;
    char buf[MaxBuf];
    vsnprintf(buf, sizeof(buf), format, args);
    ::OutputDebugStringA(buf);
    printf(buf);
#else
    vprintf(format, args);
#endif
    va_end(args);
    fflush(stdout);
}



struct TestEntry
{
    std::string name;
    std::function<void()> body;
};

static std::vector<TestEntry> g_tests;

void RegisterTestEntryImpl(const char *name, const std::function<void()>& body)
{
    g_tests.push_back({name, body});
}

static void ExecuteTest(const TestEntry& v)
{
    printf("%s begin\n", v.name.c_str());
    auto begin = Now();
    v.body();
    auto end = Now();
    printf("%s end (%fms)\n\n", v.name.c_str(), NS2MS(end-begin));
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
