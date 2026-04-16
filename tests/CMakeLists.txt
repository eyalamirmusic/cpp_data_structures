nano_add_executable(ea_data_structures_tests
    SOURCES
        VectorTests.cpp
        ArrayTests.cpp
        CircularBufferTests.cpp
        MapVectorTests.cpp
        ConstructedTests.cpp
)

target_link_libraries(ea_data_structures_tests PRIVATE ea_data_structures)
