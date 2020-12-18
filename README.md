# Examples-cpp
Examples of calling Cobalt's C++ SDKs.

## CubicExample
The /CubicExample folder contains code for calling Cobalt's Automatic Speach Recognition system, Cubic.  It is a simple client that pipes audio from an external app's stdout and calls the specified Cubic Server instance to transcribe it. 

It uses the SDK documented [here](https://sdk-cubic.cobaltspeech.com/).

## Diatheke Example
The [diatheke](./diatheke) folder contains two example clients that interact with Diatheke.
* [audio_client](./diatheke/audio_client.cpp), which is a voice only interface where the application accepts user audio, processes the result, then gives back an audio response. The audio I/O is handled by a user-specified external process, such as sox, aplay, arecord, etc.
* [cli_client](./diatheke/cli_client.cpp), which is a text only interface where the application processes text from the user, then gives a reply as text.

See [here](./diatheke/README.md) for more details about the examples, and [here](https://sdk-diatheke.cobaltspeech.com) for the SDK documentation.
