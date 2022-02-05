cmake_minimum_required(VERSION 3.16)

project(DataStructures)

add_library(ea_data_structures INTERFACE)
target_include_directories(ea_data_structures INTERFACE ${CMAKE_CURRENT_LIST_DIR})