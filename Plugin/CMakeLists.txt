CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(NormalPainterCore)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
FIND_PACKAGE(OpenEXR QUIET)
FIND_PACKAGE(FBX QUIET)

SET(ISPC "/usr/local/bin/ispc" CACHE PATH "Path to Intel ISPC")
MARK_AS_ADVANCED(FORCE ISPC)

OPTION(ENABLE_ISPC "Use Intel ISPC to generate SIMDified code. It can significantly boost performance." ON)
OPTION(ENABLE_FBX "Enable FBX support." OFF)
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
IF(ENABLE_ISPC)
    if(NOT EXISTS ${ISPC})
        # try to download ISPC
        SET(ISPC_VERSION 1.9.1)
        IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            SET(ISPC_DIR ispc-v${ISPC_VERSION}-linux)
        ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            SET(ISPC_DIR ispc-v${ISPC_VERSION}-osx)
        ENDIF()
        SET(ISPC ${CMAKE_CURRENT_SOURCE_DIR}/External/${ISPC_DIR}/ispc CACHE PATH "" FORCE)

        if(NOT EXISTS ${ISPC})
            SET(ISPC_ARCHIVE ${ISPC_DIR}.tar.gz)
            FILE(DOWNLOAD http://downloads.sourceforge.net/project/ispcmirror/v${ISPC_VERSION}/${ISPC_ARCHIVE} ${CMAKE_CURRENT_BINARY_DIR}/${ISPC_ARCHIVE} SHOW_PROGRESS)
            EXECUTE_PROCESS(
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/External
                COMMAND tar -xzvf ${CMAKE_CURRENT_BINARY_DIR}/${ISPC_ARCHIVE}
            )
        ENDIF()
    ENDIF()

    SET(MUCORE_DIR ${CMAKE_CURRENT_BINARY_DIR}/MeshUtilsCore)
    ADD_CUSTOM_TARGET(MeshUtilsCore ALL
        COMMAND ${ISPC} ${CMAKE_CURRENT_SOURCE_DIR}/MeshUtils/MeshUtilsCore.ispc -o ${MUCORE_DIR}/MeshUtilsCore${CMAKE_CXX_OUTPUT_EXTENSION} -h ${MUCORE_DIR}/MeshUtilsCore.h --pic --target=sse2,sse4,avx --arch=x86-64 --opt=fast-masked-vload --opt=fast-math
        COMMAND ${ISPC} ${CMAKE_CURRENT_SOURCE_DIR}/MeshUtils/MeshUtilsCore2.ispc -o ${MUCORE_DIR}/MeshUtilsCore2${CMAKE_CXX_OUTPUT_EXTENSION} -h ${MUCORE_DIR}/MeshUtilsCore2.h --pic --target=sse2,sse4,avx --arch=x86-64 --opt=fast-masked-vload --opt=fast-math
    )
    SET(MUCORE_FILES
        ${MUCORE_DIR}/MeshUtilsCore.h
        ${MUCORE_DIR}/MeshUtilsCore${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore_sse2${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore_sse4${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore_avx${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore2.h
        ${MUCORE_DIR}/MeshUtilsCore2${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore2_sse2${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore2_sse4${CMAKE_CXX_OUTPUT_EXTENSION}
        ${MUCORE_DIR}/MeshUtilsCore2_avx${CMAKE_CXX_OUTPUT_EXTENSION}
    )

    # create dummy files to make cmake can find it
    FOREACH(F ${MUCORE_FILES})
        FILE(WRITE ${F} "")
    ENDFOREACH(F)
ENDIF()

# MeshUtils
FILE(GLOB MU_CXX_FILES MeshUtils/*.cpp MeshUtils/*.c)
FILE(GLOB MU_H_FILES MeshUtils/*.h)
ADD_LIBRARY(MeshUtils STATIC ${MU_CXX_FILES} ${MU_H_FILES} ${MUCORE_FILES})
TARGET_INCLUDE_DIRECTORIES(MeshUtils PUBLIC ./MeshUtils)
IF(ENABLE_ISPC)
    ADD_DEFINITIONS(-DmuEnableISPC)
    ADD_DEPENDENCIES(MeshUtils MeshUtilsCore)
    TARGET_INCLUDE_DIRECTORIES(MeshUtils PUBLIC ${MUCORE_DIR})
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


# FbxExporterCore
IF(ENABLE_FBX)
    IF(APPLE)
        FIND_LIBRARY(CORE_FOUNDATION_LIBRARY CoreFoundation)
        MARK_AS_ADVANCED(FORCE CORE_FOUNDATION_LIBRARY)
    ENDIF()

    FILE(GLOB FBXE_CXX_FILES FbxExporter/*.cpp)
    FILE(GLOB FBXE_H_FILES FbxExporter/*.h)
    IF(ENABLE_OSX_BUNDLE)
        ADD_LIBRARY(FbxExporterCore MODULE ${FBXE_CXX_FILES} ${FBXE_H_FILES})
        SET_TARGET_PROPERTIES(FbxExporterCore PROPERTIES BUNDLE ON)
    ELSE()
        ADD_LIBRARY(FbxExporterCore SHARED ${FBXE_CXX_FILES} ${FBXE_H_FILES})
    ENDIF()
    ADD_DEPENDENCIES(FbxExporterCore MeshUtils)
    TARGET_INCLUDE_DIRECTORIES(FbxExporterCore PUBLIC ${FBX_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(FbxExporterCore MeshUtils ${FBX_LIBRARY} ${CORE_FOUNDATION_LIBRARY} dl)

    IF(ENABLE_DEPLOY)
        SET(FBXE_PLUGINS_DIR "${ASSETS_DIR}/UTJ/NormalPainter/Plugins/x86_64")
        IF(ENABLE_OSX_BUNDLE)
            SET(FBXE_TARGET_FILENAME "FbxExporterCore.bundle")
        ELSE()
            SET(FBXE_TARGET_FILENAME $<TARGET_FILE:FbxExporterCore>)
        ENDIF()
        ADD_CUSTOM_TARGET(DeployFbxExporterCore ALL
            COMMAND rm -rf ${FBXE_PLUGINS_DIR}/${FBXE_TARGET_FILENAME}
            COMMAND cp -r ${FBXE_TARGET_FILENAME} ${FBXE_PLUGINS_DIR}
            DEPENDS FbxExporterCore
        )
    ENDIF()
ENDIF()


# Test
IF(ENABLE_TEST)
    FILE(GLOB TEST_CXX_FILES Test/*.cpp)
    FILE(GLOB TEST_H_FILES Test/*.h)
    IF(NOT ENABLE_FBX)
        GET_FILENAME_COMPONENT(TMP Test/TestFbxExporter.cpp ABSOLUTE)
        LIST(REMOVE_ITEM TEST_CXX_FILES ${TMP})
    ENDIF()

    ADD_EXECUTABLE(Test ${TEST_CXX_FILES} ${TEST_H_FILES})
    ADD_DEPENDENCIES(Test MeshUtils)
    TARGET_LINK_LIBRARIES(Test MeshUtils)
    IF(ENABLE_FBX)
        ADD_DEPENDENCIES(Test FbxExporterCore)
        TARGET_LINK_LIBRARIES(Test FbxExporterCore)
    ENDIF()
ENDIF()
