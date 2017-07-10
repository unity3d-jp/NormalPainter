if(ENABLE_TEST)
    file(GLOB TEST_SOURCES *.cpp)
    file(GLOB TEST_HEADERS *.h)

    add_executable(Test ${TEST_SOURCES} ${TEST_HEADERS})
    add_dependencies(Test MeshUtils)
    target_link_libraries(Test MeshUtils)
    install(TARGETS Test DESTINATION .)
endif()
