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

static void RunTestImpl(const TestEntry& v)
{
    Print("%s begin\n", v.name.c_str());
    auto begin = Now();
    v.body();
    auto end = Now();
    Print("%s end (%.2fms)\n\n", v.name.c_str(), NS2MS(end-begin));
}

testExport void RunTest(char *name)
{
    for (auto& entry : g_tests) {
        if (entry.name == name) {
            RunTestImpl(entry);
        }
    }
}

testExport void RunAllTests()
{
    for (auto& entry : g_tests) {
        RunTestImpl(entry);
    }
}

int main(int argc, char *argv[])
{
    if (argc == 1) {
        RunAllTests();
    }
    else {
        for (int i = 1; i < argc; ++i) {
            RunTest(argv[i]);
        }
    }

    printf("Press any key to exit\n");
    char dummy;
    scanf("%c", &dummy);
}
