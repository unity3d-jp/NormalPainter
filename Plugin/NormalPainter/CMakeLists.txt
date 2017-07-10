file(GLOB NP_SOURCES *.cpp)
file(GLOB NP_HEADERS *.h)
if(ENABLE_OSX_BUNDLE)
    add_library(NormalPainterCore MODULE ${NP_SOURCES} ${NP_HEADERS})
    set_target_properties(NormalPainterCore PROPERTIES BUNDLE ON)
else()
    add_library(NormalPainterCore SHARED ${NP_SOURCES} ${NP_HEADERS})
endif()
add_dependencies(NormalPainterCore MeshUtils)
target_link_libraries(NormalPainterCore MeshUtils)
install(TARGETS NormalPainterCore DESTINATION .)

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
