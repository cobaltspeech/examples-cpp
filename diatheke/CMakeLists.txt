cmake_minimum_required(VERSION 3.14.0)
project(diatheke_examples)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Download the SDK from github and add it as part of the project
include(FetchContent)
FetchContent_Declare(
  sdk_diatheke
  GIT_REPOSITORY https://github.com/cobaltspeech/sdk-diatheke.git
  GIT_TAG        v2.1.0
)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(sdk_diatheke)
add_subdirectory(${sdk_diatheke_SOURCE_DIR}/grpc/cpp-diatheke ${sdk_diatheke_BINARY_DIR})

# Build the text-only CLI and link against the Diatheke SDK.
add_executable(cli_client cli_client.cpp)
target_link_libraries(cli_client PRIVATE diatheke_client)

# Build the voice-only interface
add_executable(audio_client
  audio_client.cpp
  recorder.cpp
  recorder.h
  player.cpp
  player.h
)

# Link against the Diatheke SDK.
target_link_libraries(audio_client PRIVATE diatheke_client)
target_include_directories(audio_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
