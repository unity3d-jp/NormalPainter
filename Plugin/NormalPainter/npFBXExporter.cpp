#include "pch.h"
#include "NormalPainter.h"

#ifdef npEnableFBX
#include <fbxsdk.h>
#ifdef _WIN32
    #pragma comment(lib, "libfbxsdk-md.lib")
#endif

#endif // npEnableFBX
