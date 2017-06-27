#pragma once

void RegisterTestEntryImpl(const char *name, const std::function<void()>& body);

#define RegisterTestEntry(Name)\
    struct Register##Name {\
        Register##Name() { RegisterTestEntryImpl(#Name, Name); }\
    } g_Register##Name;
