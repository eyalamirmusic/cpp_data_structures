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
        Flags/BoolTests.cpp
        Flags/CopyableAtomicTests.cpp
        Flags/LocksTests.cpp
        Flags/RecursiveSpinLockTests.cpp
        Pointers/AnyTests.cpp
        Pointers/CallbackFuncTests.cpp
        Pointers/CloneableTests.cpp
        Pointers/OwningPointerTests.cpp
        Pointers/RefOrOwnTests.cpp
        Pointers/RefTests.cpp
        Structures/ArrayTests.cpp
        Structures/BufferViewTests.cpp
        Structures/CircularBufferTests.cpp
        Structures/CopyOnWriteTests.cpp
        Structures/FifoTests.cpp
        Structures/FilteredTests.cpp
        Structures/FixedDynamicArrayTests.cpp
        Structures/MapVectorTests.cpp
        Structures/MultiVectorTests.cpp
        Structures/OwnedVectorTests.cpp
        Structures/SharedGUIDataTests.cpp
        Structures/SmallVectorTests.cpp
        Structures/StaticVectorTests.cpp
        Structures/VectorTests.cpp
        Utilities/GenericUtilitiesTests.cpp
        Utilities/MapUtilitiesTests.cpp
        Utilities/StaticObjectsTests.cpp
        Utilities/TupleUtilitiesTests.cpp
        Utilities/VectorUtilitiesTests.cpp
        ValueWrapper/ConstructedTests.cpp
        ValueWrapper/RawStorageTests.cpp
        ValueWrapper/ValueTests.cpp
)

target_link_libraries(ea_data_structures_tests PRIVATE ea_data_structures)

target_include_directories(ea_data_structures_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(ea_data_structures_tests PRIVATE
    $<$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>:/W4 /WX>
    $<$<CXX_COMPILER_FRONTEND_VARIANT:GNU>:-Wall -Wextra -Wpedantic -Werror>
)
