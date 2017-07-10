file(GLOB NP_SOURCES *.cpp)
file(GLOB NP_HEADERS *.h)
set(NP_PLUGINS_DIR "${CMAKE_SOURCE_DIR}/../NormalPainter/Assets/UTJ/NormalPainter/Plugins/x86_64")
add_plugin(NormalPainterCore SOURCES ${NP_SOURCES} ${NP_HEADERS} PLUGINS_DIR ${NP_PLUGINS_DIR})

add_dependencies(NormalPainterCore MeshUtils)
target_link_libraries(NormalPainterCore MeshUtils ${EXTERNAL_LIBS})
install(TARGETS NormalPainterCore DESTINATION .)
