cmake_minimum_required(VERSION 3.23.2)
project(mkr_glsl_include_test)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set Executable Directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# Source Files
set(UT_DIR "ut")
file(GLOB_RECURSE UT_FILES LIST_DIRECTORIES true CONFIGURE_DEPENDS
        "${UT_DIR}/*.h"
        "${UT_DIR}/*.c"
        "${UT_DIR}/*.hpp"
        "${UT_DIR}/*.cpp")
add_executable(${PROJECT_NAME} ${UT_FILES})

# External Dependencies
include(FetchContent)
FetchContent_Declare(google_test GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG main)
FetchContent_MakeAvailable(google_test)

# Target
target_link_libraries(${PROJECT_NAME} PUBLIC gtest_main mkr_glsl_include)

# Test
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
