#pragma once

#ifdef _WIN32
    #define npAPI extern "C" __declspec(dllexport)
    //#define npEnablePenTablet
    //#define npEnableFBX
#else
    #define npAPI extern "C" 
#endif

