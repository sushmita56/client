cmake_minimum_required(VERSION 3.16)

# set the project name
project(ContractTesting LANGUAGES CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(EXECUTABLE_NAME contracts)

set(TEST_SRCS
  pactHelper.cpp
)

add_executable(${EXECUTABLE_NAME} ${TEST_SRCS})

# link libraries
target_link_libraries(${EXECUTABLE_NAME} ${CONAN_LIBS})
