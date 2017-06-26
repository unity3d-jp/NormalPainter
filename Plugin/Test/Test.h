#pragma once

struct TestEntry
{
    std::string name;
    std::function<void()> body;
};

void RegisterTestEntryImpl(const TestEntry& v);

#define RegisterTestEntry(Name)\
    struct Register##Name {\
        Register##Name() {\
            TestEntry entry {#Name, Name};\
            RegisterTestEntryImpl(entry);\
        }\
    } g_Register##Name;
