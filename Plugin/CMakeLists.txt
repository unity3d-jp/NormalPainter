CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(NormalPainterCore)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
INCLUDE(ISPC)
FIND_PACKAGE(OpenEXR QUIET)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE PATH "" FORCE)
endif()
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "dist" CACHE PATH "" FORCE)
endif()

SET(ISPC "/usr/local/bin/ispc" CACHE PATH "Path to Intel ISPC")
MARK_AS_ADVANCED(FORCE ISPC)

OPTION(ENABLE_ISPC "Use Intel ISPC to generate SIMDified code. It can significantly boost performance." ON)
OPTION(ENABLE_DEPLOY "Copy built binaries to plugins directory." ON)
OPTION(ENABLE_TEST "Build tests" OFF)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11 -Wno-deprecated")

IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    OPTION(ENABLE_OSX_BUNDLE "Build bundle." ON)
    SET(CMAKE_MACOSX_RPATH ON)

    IF(ENABLE_OSX_BUNDLE)
        SET(CMAKE_SKIP_RPATH ON)
    ELSE()
        SET(CMAKE_SKIP_RPATH OFF)
    ENDIF()
ENDIF()

SET(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../NormalPainter/Assets")
INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(MeshUtils)
add_subdirectory(NormalPainter)
add_subdirectory(Test)
