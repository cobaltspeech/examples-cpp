# Copyright (2019) Cobalt Speech and Language, Inc.

cmake_minimum_required(VERSION 3.5.0)
project(cubic-cpp-examples)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# Add the cubic client code as a subdirectory
add_subdirectory(sdk-cubic/grpc/cpp-cubic cpp-cubic)

# Add third party dependencies
add_subdirectory(thirdParty)


# Demo using ASR via Diatheke
add_executable(demo_asr 
    demo_config.cpp
    demo_config.h
    main.cpp
    recorder.cpp
    recorder.h
    timer.cpp
    timer.h)

target_link_libraries(demo_asr PRIVATE
    cubic_client
    cpptoml)

