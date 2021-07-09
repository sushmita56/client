cmake_minimum_required(VERSION 3.16)

# set the project name
project(ContractTesting LANGUAGES CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# libraries to include
set(INCLUDE_LIBS
  ${GTEST_LIBRARIES}
  pact-cpp-consumer
  pact_mock_server_ffi
  pthread
)

set(EXECUTABLE_NAME contracts)

set(TEST_SRCS
  pactHelper.cpp
)

add_executable(${EXECUTABLE_NAME} ${TEST_SRCS})

# link libraries
target_link_libraries(${EXECUTABLE_NAME} ${INCLUDE_LIBS})
