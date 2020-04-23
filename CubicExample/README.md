# C++ Examples

This directory contains an example showing how to use the Cubic C++ SDK.

## Build
The example uses [CMake](www.cmake.org) as the build system. To compile,
do the following, assuming that the variable $SRC refers to this directory
and $BUILD refers to the directory where you the binary should be built. 

```bash
# Download the sdk-cubic repo to this directory
cd $SRC
git clone https://github.com/cobaltspeech/sdk-cubic.git

# Create the build directory if it doesn't exist and move to it.
mkdir -p $BUILD
cd $BUILD

# Run CMake. This will also download dependencies (cpptoml and gRPC)
cmake -DCMAKE_BUILD_TYPE=Release $SRC

# Compile the project
make -j <num cpu>
```

This will create an executable named `demo_asr` in the build directory.


# Run
Before starting the demo app, first ensure that both Cubic server is running.
Then make sure the demo app can use it by specifying the address and port
number in the demo's configuration file (see examples/config.sample.toml for
a sample config file).

Start the application and provide a configuration file
```bash
./demo_asr -config <path/to/config.toml>
```

Once the demo is running, the user will be prompted to press the Enter key to
start and stop recording. Transcriptions and other statistics will be displayed
as they are available.

# Audio I/O
Rather than having a tight integration with specific audio drivers, all the audio I/O for this demo is handled by an external application. Any application may be used as long as the following requirement is met:

* Recording - the recording application must send the recorded audio samples
  to stdout. The samples should be in a format that the Cubic model understands
  (e.g., 16-bit raw PCM data at 16kHz sample rate). Two such applications that
  are known to work for this demo are [sox](http://sox.sourceforge.net/)
  (multi-platform) and [arecord](https://linux.die.net/man/1/arecord) (Linux).

The recording application must be specified in the config file for the
demo to work properly.
