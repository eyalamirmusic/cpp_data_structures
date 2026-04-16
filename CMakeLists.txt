cmake_minimum_required(VERSION 3.21)

project(DataStructures CXX)

add_library(ea_data_structures INTERFACE)
target_include_directories(ea_data_structures INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_compile_features(ea_data_structures INTERFACE cxx_std_20)

if(PROJECT_IS_TOP_LEVEL)
    enable_testing()
    add_subdirectory(tests)
endif()
