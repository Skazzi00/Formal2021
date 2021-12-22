cmake_minimum_required(VERSION 3.16)
project(Formal2021)

set(CMAKE_CXX_STANDARD 20)
file(COPY ${CMAKE_SOURCE_DIR}/etc/NFA_test DESTINATION .)

set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)
include_directories(${COMMON_INCLUDES})

SET(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
# SET(GCC_COVERAGE_LINK_FLAGS    "-lgcov")

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")


add_subdirectory(./googletest)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)
add_executable(runUnitTests ${TEST_SRC_FILES})

# Standard linking to gtest stuff
target_link_libraries(runUnitTests gtest gtest_main)
add_test(NAME runUnitTests COMMAND runUnitTests)

add_executable(eNFAtoDFA src/eNFAtoDFA.cpp)
add_executable(hw5 src/hw5.cpp)

