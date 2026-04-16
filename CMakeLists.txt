cmake_minimum_required(VERSION 3.21)

project(DataStructures CXX)

add_library(ea_data_structures INTERFACE)
target_include_directories(ea_data_structures INTERFACE ${CMAKE_CURRENT_LIST_DIR})

if(PROJECT_IS_TOP_LEVEL)
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

    enable_testing()
    add_subdirectory(tests)
endif()
