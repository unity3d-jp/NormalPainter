set(CMAKE_PREFIX_PATH
    "/Applications/Autodesk/FBX SDK/2018.1.1"
    "/Applications/Autodesk/FBX SDK/2017.1"
    "/Applications/Autodesk/FBX SDK/2016.1.2"
    "/opt/Autodesk/FBX SDK/2018.1.1"
    "/opt/Autodesk/FBX SDK/2017.1"
    "/opt/Autodesk/FBX SDK/2016.1.2"
)

find_path(FBX_INCLUDE_DIR
    NAMES fbxsdk.h
)

find_library(FBX_LIBRARY
    NAMES fbxsdk
    PATH_SUFFIXES lib lib/clang/release
)

mark_as_advanced(FBX_INCLUDE_DIR)
mark_as_advanced(FBX_LIBRARY)

find_package_handle_standard_args("FBX"
    DEFAULT_MSG
    FBX_LIBRARY
    FBX_INCLUDE_DIR
)
