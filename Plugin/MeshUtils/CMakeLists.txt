if(ENABLE_ISPC)
    setup_ispc()

    set(MUISPC_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/ISPC)
    set(headers
        "${CMAKE_CURRENT_SOURCE_DIR}/ispcmath.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/muSIMDConfig.h"
    )
    set(sources
        "${CMAKE_CURRENT_SOURCE_DIR}/MeshUtilsCore.ispc"
        "${CMAKE_CURRENT_SOURCE_DIR}/MeshUtilsCore2.ispc"
    )
    add_ispc_targets(SOURCES ${sources} HEADERS ${headers} OUTDIR ${MUISPC_OUTDIR})
    set(MUISPC_OUTPUTS ${_ispc_headers} ${_ispc_objects})
endif()

file(GLOB MU_SOURCES *.cpp *.c)
file(GLOB MU_HEADERS *.h)
add_library(MeshUtils STATIC ${MU_SOURCES} ${MU_HEADERS} ${MUISPC_OUTPUTS})
install(TARGETS MeshUtils DESTINATION .)

if(ENABLE_ISPC)
    add_definitions(-DmuEnableISPC)
    target_include_directories(MeshUtils PUBLIC ${MUISPC_OUTDIR})
endif()
