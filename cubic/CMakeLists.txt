# Copyright (2021) Cobalt Speech and Language, Inc.

cmake_minimum_required(VERSION 3.14.0)
project(cubic-cpp-examples)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# Download the SDK from github and add it as part of the project
include(FetchContent)
FetchContent_Declare(
  sdk_cubic
  GIT_REPOSITORY https://github.com/cobaltspeech/sdk-cubic.git
  GIT_TAG        master
)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(sdk_cubic)
add_subdirectory(${sdk_cubic_SOURCE_DIR}/grpc/cpp-cubic ${sdk_cubic_BINARY_DIR})


# Create demos
add_executable(synchronous_client synchronous_client.cpp)
target_link_libraries(synchronous_client PRIVATE cubic_client)

add_executable(stream_client stream_client.cpp)
target_link_libraries(stream_client PRIVATE cubic_client)

add_executable(mic_client
   mic_client.cpp
   recorder.cpp
   recorder.h
)
target_link_libraries(mic_client PRIVATE cubic_client)

add_executable(context_client context_client.cpp)
target_link_libraries(context_client PRIVATE cubic_client)
