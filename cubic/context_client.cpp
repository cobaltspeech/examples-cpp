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

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

/*
 * Create some aliases to make the code more readable. The gRPC
 * interface can be a bit verbose.
 */
namespace CubicPB = cobaltspeech::cubic;

// Some useful variables to define the client configuration
const std::string serverAddress = "localhost:2727";
const std::string filename = "test.wav";

// This client demonstrates using compiled contexts with streaming
// recognition.
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
            std::cout << "  ID = " << m.id() << std::endl
                      << "  Name = " << m.name()
                      << "  Supports Context = " << m.supportsContext() << std::endl;
            
            if (m.supportsContext()) {
                std::vector<std::string> contextTokens = m.allowedContextTokens();
                std::string tokString;
                for (const std::string &tok : contextTokens) {
                    tokString += tok + ", ";
                }
                tokString.erase(tokString.end() - 2, tokString.end());

                std::cout << "  Allowed Context Tokens = " << tokString << std::endl;
            }

            std::cout << std::endl;
        }

        // Assuming the first model supports context
        auto model = models[0];

        // Let's say this model has an allowed context token called
        // "airport_names" and we have a list of airport names that
        // we ant to make sure the recognizer gets right. We compile
        // the list of names using the CompileContext() method, save
        // the compiled data and send it with subsequent recognize
        // requests to customize and improve the results.
        std::vector<std::string> phrases {
            "NARITA",
            "KUALA LUMPUR INTERNATIONAL",
            "ISTANBUL ATATURK",
            "LAGUARDIA"
        };

        std::string contextToken = model.allowedContextTokens()[0]; // "airport_names"
        CubicPB::CompiledContext compiledResp =
            client.compileContext(model.id(), contextToken, phrases);

        // Save the compiled result for later use. Note this compiled
        // data is only compatible with the model ID used to create it.
        CubicPB::RecognitionContext compiledContexts;
        *(compiledContexts.add_compiled()) = compiledResp;

        // Now we can send a recognize request along with the compiled
        // context. The context data is provided throught he recognition
        // config.
        CubicPB::RecognitionConfig cfg;
        cfg.set_model_id(model.id());
        cfg.set_audio_encoding(CubicPB::RecognitionConfig::WAV);
        *(cfg.mutable_context()) = compiledContexts;

        // The rest is the same as the usual streaming recognize request.

        // Create the stream
        auto stream = client.streamingRecognize(cfg);

        // Push the audio on a separate thread
        std::thread audioThread([&stream](){
            // Open the file and push audio bytes
            std::ifstream infile(filename);
            std::streamsize buffSize = 8192;
            char *buff = new char[buffSize];
            while (infile.good()) {
                infile.read(buff, buffSize);
                stream.pushAudio(buff, infile.gcount());
            }

            // Let Cubic know that no more audio will be coming
            stream.audioFinished();
            delete[] buff;
        });

        // Print the results as they come
        std::cout << "\nTranscripts:" << std::endl;
        CubicPB::RecognitionResponse resp;
        while (stream.receiveResults(&resp)) {
            for (int i = 0; i < resp.results_size(); i++) {
                CubicPB::RecognitionResult result = resp.results(i);
                if (!result.is_partial()) {
                    std::cout << result.alternatives(0).transcript() << std::endl;
                }
            }
        }

        // Close the stream
        audioThread.join();
        stream.close();

    } catch (CubicException &e) {
        std::cerr << "Cubic error: " << e.what() << std::endl;
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    std::cout << "\nDone." << std::endl;
}
