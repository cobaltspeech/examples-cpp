# Examples-cpp
Examples of calling Cobalt's C++ SDKs.

## CubicExample
The [cubic](./cubic) folder contains four example clients for calling Cobalt's Automatic Speach Recognition system, Cubic.
* [synchronous_client](./cubic/synchronous_client.cpp), which demonstrates synchronous speech recognition.
* [stream_client](./cubic/stream_client.cpp), which demonstrates streaming speech recognition.
* [context_client](./cubic/context_client.cpp), which demonstrates streaming ASR using context lists to improve speech recognition for specific words or phrases.
* [mic_client](./cubic/mic_client.cpp), which demonstrates streaming ASR using a microphone for audio input.

See [here](./cubic/README.md) for more details about the examples, and [here](https://sdk-cubic.cobaltspeech.com/) for the SDK documentation.

## Diatheke Example
The [diatheke](./diatheke) folder contains two example clients that interact with Diatheke.
* [audio_client](./diatheke/audio_client.cpp), which is a voice only interface where the application accepts user audio, processes the result, then gives back an audio response. The audio I/O is handled by a user-specified external process, such as sox, aplay, arecord, etc.
* [cli_client](./diatheke/cli_client.cpp), which is a text only interface where the application processes text from the user, then gives a reply as text.

See [here](./diatheke/README.md) for more details about the examples, and [here](https://sdk-diatheke.cobaltspeech.com) for the SDK documentation.
