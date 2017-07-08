#pragma once

void RegisterTestEntryImpl(const char *name, const std::function<void()>& body);
void PrintImpl(const char *format, ...);

#define RegisterTestEntry(Name)\
    struct Register##Name {\
        Register##Name() { RegisterTestEntryImpl(#Name, Name); }\
    } g_Register##Name;

#define Print(...) PrintImpl(__VA_ARGS__)

#ifdef _WIN32
    #define testExport extern "C" __declspec(dllexport)
#else
    #define testExport extern "C" 
#endif

