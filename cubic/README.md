# Cubic Examples
This directory contains code demonstrating how to use the [Cubic SDK](https://sdk-cubic.cobaltspeech.com).

## Build
Like the SDK, these examples use [CMake](https://cmake.org/) to manage the build process. CMake will automatically download the correct version of the SDK and the gRPC code required to use the SDK.

```bash
# Create the build directory
mkdir build

# Run CMake from the build directory. The first run will take a
# little while as it downloads the SDK and gRPC code.
cd build
cmake <path/to/examples-cpp/cubic>

# Use the CMake command to build the examples
cmake --build .

# OR call make directly
make -j8
```

Because the code must also build the gRPC library, the build process will likely take somewhere between 10-15 minutes (depending on hardware).

## Run
These examples are intended to be run from the command line. Note that for these exampels, the server address, model ID, and other options are hardcoded.

```bash
cd <path/to/build>

# Run one of the following the compiled clients
./synchronous_client
./stream_client
./context_client
./mic_client
```

Note that all of the examples, except the `mic_client`, expect a file named "test.wav" or "test.raw" to be in the current working directory when the application is launched. This directory contains two example audio files for convenience.

For the `mic_client` example, the audio input is handled by an external application such as arecord or sox. The specific application can be anything as long as the following conditions are met.
* The application supports the encodings, sample rate, bit-depth, etc. required by the underlying Cubic ASR models.
* The application must stream audio data to stdout.

The specific applicaiton (and their args) should be specified as strings in the code (the `recordCmd` variable). When integrating the Cubic SDK with your application, it is recommended to use your preferred C++ library to handle the audio I/O.
