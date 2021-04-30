/*
 * Copyright (2021) Cobalt Speech and Language, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cubic_client.h"
#include "cubic_exception.h"
#include "recorder.h"

#include <atomic>
#include <iostream>
#include <string>
#include <thread>

/*
 * Create some aliases to make the code more readable. The gRPC
 * interface can be a bit verbose.
 */
namespace CubicPB = cobaltspeech::cubic;

// Some useful variables to define the client configuration
const std::string serverAddress = "localhost:2727";
const std::string modelID = "1";

// The external process responsible for recording audio.
const std::string recordCmd =
    "sox -q -d -c 1 -r 16000 -b 16 -L -e signed -t raw -";

// Wait for the Enter key to be pressed
void waitForEnter() {
    // This is a somewhat simplistic way to detect if the enter key was
    // pressed, but it is sufficient for demo purposes.
    std::string line_input;
    std::getline(std::cin, line_input);
}

// This client demonstrates using streaming recognition with a
// microphone, with the audio data being piped from an external
// process. The external process is used for convenience. Any
// audio library that can access the microphone device may be
// used instead.
int main(int argc, char *argv[]) {
    try {
        // Create the client (note this is an insecure connection,
        // which is not recommended for production).
        CubicClient client(serverAddress);

        // Display the Cubic version
        std::cout << "Cubic version: " << client.cubicVersion() << std::endl;
        std::cout << "Server version: " << client.serverVersion() << std::endl;
        std::cout << "Connected to " << serverAddress << std::endl;
        std::cout << std::endl;

        // Get the list of available models
        std::vector<CubicModel> models = client.listModels();
        std::cout << "Available Models:" << std::endl;
        for (const CubicModel &m : models) {
            std::cout << "ID = " << m.id() << ", Name = " << m.name() << std::endl;
        }

        // Use the first model to set up the recognition config
        auto modelID = models[0].id();
        CubicPB::RecognitionConfig cfg;
        cfg.set_model_id(modelID);
        cfg.set_audio_encoding(CubicPB::RecognitionConfig::RAW_LINEAR16);

        // Create the stream
        auto stream = client.streamingRecognize(cfg);

        // Handle the microphone audio on a separate thread
        std::atomic_bool isRecording(true);
        std::thread audioThread([&stream, &isRecording](){
           // Start recording
           Recorder rec(recordCmd);
           rec.start();

           // Push the recorded audio to Cubic
            while(isRecording) {
                std::string audio = rec.readAudio();
                stream.pushAudio(audio.c_str(), audio.length());
            }

            // Let Cubic know that no more audio will be coming
            rec.stop();
            stream.audioFinished();
        });

        // Print the results as they come on a separate thread
        std::thread resultsThread([&stream]() {
            CubicPB::RecognitionResponse resp;
            while (stream.receiveResults(&resp)) {
                for (int i = 0; i < resp.results_size(); i++) {
                    CubicPB::RecognitionResult result = resp.results(i);
                    if (!result.is_partial()) {
                        std::cout << result.alternatives(0).transcript() << std::endl;
                    }
                }
            }
        });

        // On the main thread, wait for the user to press Enter
        std::cout << "\n\nRecording. Press Enter to stop." <<
                     "\n\nTranscripts:" << std::endl;

        waitForEnter();
        isRecording = false;
        audioThread.join();
        resultsThread.join();
        stream.close();

    } catch (CubicException &e) {
        std::cerr << "Cubic error: " << e.what() << std::endl;
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    std::cout << "\nDone." << std::endl;
}
