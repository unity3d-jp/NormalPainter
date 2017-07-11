if(APPLE)
    option(ENABLE_TEST "Build tests" OFF)
else()
    option(ENABLE_TEST "Build tests" ON)
endif()

if(ENABLE_TEST)
    file(GLOB TEST_SOURCES *.cpp)
    file(GLOB TEST_HEADERS *.h)

    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}")

    add_executable(Test ${TEST_SOURCES} ${TEST_HEADERS})
    add_dependencies(Test MeshUtils)
    target_link_libraries(Test MeshUtils ${EXTERNAL_LIBS})
    install(TARGETS Test DESTINATION .)
endif()
