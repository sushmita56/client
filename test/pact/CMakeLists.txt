cmake_minimum_required(VERSION 3.16)

# set the project name
project(pact-cpp-consumer VERSION 0.0.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# # Initialize Conan
# INCLUDE(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
# CONAN_BASIC_SETUP()
 
# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
 
# Link runTests with what we want to test and the GTest and pthread library
add_executable(runTests include/consumer.cpp pactHelper.cpp)
target_link_libraries(runTests ${GTEST_LIBRARIES} pthread)
