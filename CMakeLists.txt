cmake_minimum_required (VERSION 3.9)
MESSAGE(STATUS "CMAKE_ROOT: " ${CMAKE_ROOT})

# Project name
project(cadI_project)

include(CTest)

# Turn on the verbose
set(CMAKE_VERBOSE_MAKEFILE ON)

# CXX target properties
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(USE_WERROR ON)
#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror -O0 -fopenmp -ltcl8.6 ")
message(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
message(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER})
message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_EXE_LINKER_FLAGS: " ${CMAKE_EXE_LINKER_FLAGS})


# add the binary tree to the search path for include files
include_directories(${PROJECT_SOURCE_DIR})
include_directories(src-liberty_parse-2.6)
include_directories(structs)
include_directories(lib)
include_directories(.)

# -----------------------------------------------------------------------------
# Example program
# -----------------------------------------------------------------------------
message(STATUS "Building examples ...")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)

message(STATUS "EXAMPLE_CXX_FLAGS: " ${EXAMPLE_CXX_FLAGS})
message(STATUS "EXAMPLE_EXE_LINKER_FLAGS: " ${EXAMPLE_EXE_LINKER_FLAGS})

# LIB parser
add_custom_command ( OUTPUT src-liberty_parse-2.6/liblibparse.a
                     COMMAND make
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src-liberty_parse-2.6/)
add_custom_target(
                     liblibparse ALL
                     DEPENDS src-liberty_parse-2.6/liblibparse.a
                   )

link_directories(src-liberty_parse-2.6)

# ASP
link_directories(libasp)

add_executable(cadI_project cadI_project.cpp structs/structs.cpp structs/structs_unit.cpp lib/lib.cpp)


target_link_libraries(cadI_project PRIVATE asp libparse tcl8.6 pthread dl m)
# target_link_libraries(cadI_project asp libparse)
