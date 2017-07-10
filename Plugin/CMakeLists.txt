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

add_subdirectory(MeshUtils)


# NormalPainterCore
file(GLOB NP_SOURCES NormalPainter/*.cpp)
file(GLOB NP_HEADERS NormalPainter/*.h)
if(ENABLE_OSX_BUNDLE)
    add_library(NormalPainterCore MODULE ${NP_SOURCES} ${NP_HEADERS})
    set_target_properties(NormalPainterCore PROPERTIES BUNDLE ON)
else()
    add_library(NormalPainterCore SHARED ${NP_SOURCES} ${NP_HEADERS})
endif()
add_dependencies(NormalPainterCore MeshUtils)
target_link_libraries(NormalPainterCore MeshUtils)

if(ENABLE_DEPLOY)
    set(NP_PLUGINS_DIR "${ASSETS_DIR}/UTJ/NormalPainter/Plugins/x86_64")
    if(ENABLE_OSX_BUNDLE)
        SET(NP_TARGET_FILENAME "NormalPainterCore.bundle")
    else()
        SET(NP_TARGET_FILENAME $<TARGET_FILE:NormalPainterCore>)
    endif()
    add_custom_target(DeployNormalPainterCore ALL
        COMMAND rm -rf ${NP_PLUGINS_DIR}/${NP_TARGET_FILENAME}
        COMMAND cp -r ${NP_TARGET_FILENAME} ${NP_PLUGINS_DIR}
        DEPENDS NormalPainterCore
    )
endif()


# Test
if(ENABLE_TEST)
    file(GLOB TEST_SOURCES Test/*.cpp)
    file(GLOB TEST_HEADERS Test/*.h)

    add_executable(Test ${TEST_SOURCES} ${TEST_HEADERS})
    add_dependencies(Test MeshUtils)
    target_link_libraries(Test MeshUtils)
endif()

