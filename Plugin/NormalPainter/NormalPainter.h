#pragma once

#ifdef _WIN32
    #define npAPI extern "C" __declspec(dllexport)
    //#define npEnablePenTablet
#else
    #define npAPI extern "C" 
#endif

