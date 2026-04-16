set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    NanoTest
    GIT_REPOSITORY https://github.com/eyalamirmusic/NanoTest.git
    GIT_TAG main
)
FetchContent_MakeAvailable(NanoTest)

nano_add_executable(ea_data_structures_tests
    SOURCES
        VectorTests.cpp
        ArrayTests.cpp
        CircularBufferTests.cpp
        MapVectorTests.cpp
        ConstructedTests.cpp
        RawStorageTests.cpp
        PointersTests.cpp
        FlagsTests.cpp
        ValueTests.cpp
        UtilitiesTests.cpp
        StaticVectorTests.cpp
        SmallVectorTests.cpp
        OwnedVectorTests.cpp
        BufferViewTests.cpp
        FixedDynamicArrayTests.cpp
        CopyOnWriteTests.cpp
        MultiVectorTests.cpp
        FifoTests.cpp
        FilteredTests.cpp
        TupleTests.cpp
)

target_link_libraries(ea_data_structures_tests PRIVATE ea_data_structures)

target_compile_options(ea_data_structures_tests PRIVATE
    $<$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>:/W4 /WX>
    $<$<CXX_COMPILER_FRONTEND_VARIANT:GNU>:-Wall -Wextra -Wpedantic -Werror>
)
