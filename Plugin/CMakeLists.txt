CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(NormalPainterCore)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
INCLUDE(ISPC)
FIND_PACKAGE(OpenEXR QUIET)

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

# MeshUtilsCore
set(MU_DIR "${CMAKE_CURRENT_SOURCE_DIR}/MeshUtils")
IF(ENABLE_ISPC)
    setup_ispc()

    SET(MUCORE_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/MeshUtilsCore)
    SET(headers
        "${MU_DIR}/ispcmath.h"
        "${MU_DIR}/muSIMDConfig.h"
    )
    SET(sources
        "${MU_DIR}/MeshUtilsCore.ispc"
        "${MU_DIR}/MeshUtilsCore2.ispc"
    )
    foreach(source ${sources})
        add_ispc_target(${source} DEPENDS ${headers} OUTDIR ${MUCORE_OUTDIR})
        list(APPEND MUCORE_FILES ${_ispc_outputs})
    endforeach()
ENDIF()

# MeshUtils
FILE(GLOB MU_CXX_FILES MeshUtils/*.cpp MeshUtils/*.c)
FILE(GLOB MU_H_FILES MeshUtils/*.h)
ADD_LIBRARY(MeshUtils STATIC ${MU_CXX_FILES} ${MU_H_FILES} ${MUCORE_FILES})
TARGET_INCLUDE_DIRECTORIES(MeshUtils PUBLIC ./MeshUtils)
IF(ENABLE_ISPC)
    ADD_DEFINITIONS(-DmuEnableISPC)
    TARGET_INCLUDE_DIRECTORIES(MeshUtils PUBLIC ${MUCORE_OUTDIR})
ENDIF()


# NormalPainterCore
FILE(GLOB NP_CXX_FILES NormalPainter/*.cpp)
FILE(GLOB NP_H_FILES NormalPainter/*.h)
IF(ENABLE_OSX_BUNDLE)
    ADD_LIBRARY(NormalPainterCore MODULE ${NP_CXX_FILES} ${NP_H_FILES})
    SET_TARGET_PROPERTIES(NormalPainterCore PROPERTIES BUNDLE ON)
ELSE()
    ADD_LIBRARY(NormalPainterCore SHARED ${NP_CXX_FILES} ${NP_H_FILES})
ENDIF()
ADD_DEPENDENCIES(NormalPainterCore MeshUtils)
TARGET_INCLUDE_DIRECTORIES(NormalPainterCore PUBLIC ./NormalPainter)
TARGET_LINK_LIBRARIES(NormalPainterCore MeshUtils)

IF(ENABLE_DEPLOY)
    SET(NP_PLUGINS_DIR "${ASSETS_DIR}/UTJ/NormalPainter/Plugins/x86_64")
    IF(ENABLE_OSX_BUNDLE)
        SET(NP_TARGET_FILENAME "NormalPainterCore.bundle")
    ELSE()
        SET(NP_TARGET_FILENAME $<TARGET_FILE:NormalPainterCore>)
    ENDIF()
    ADD_CUSTOM_TARGET(DeployNormalPainterCore ALL
        COMMAND rm -rf ${NP_PLUGINS_DIR}/${NP_TARGET_FILENAME}
        COMMAND cp -r ${NP_TARGET_FILENAME} ${NP_PLUGINS_DIR}
        DEPENDS NormalPainterCore
    )
ENDIF()


# Test
IF(ENABLE_TEST)
    FILE(GLOB TEST_CXX_FILES Test/*.cpp)
    FILE(GLOB TEST_H_FILES Test/*.h)

    ADD_EXECUTABLE(Test ${TEST_CXX_FILES} ${TEST_H_FILES})
    ADD_DEPENDENCIES(Test MeshUtils)
    TARGET_LINK_LIBRARIES(Test MeshUtils)
ENDIF()
