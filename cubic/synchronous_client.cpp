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

/*
 * Create some aliases to make the code more readable. The gRPC
 * interface can be a bit verbose.
 */
namespace CubicPB = cobaltspeech::cubic;

// Some useful variables to define the client configuration
const std::string serverAddress = "localhost:2727";
const std::string filename = "test.wav";

// This client demonstrates using synchronous recognition.
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
        cfg.set_audio_encoding(CubicPB::RecognitionConfig::WAV);

        // Read the entire audio file
        std::ifstream infile(filename);
        std::string data( (std::istreambuf_iterator<char>(infile)),
                          (std::istreambuf_iterator<char>()) );

        // Send the recognition request.
        CubicPB::RecognitionResponse resp = client.recognize(cfg, data.c_str(), data.length());

        // Print the results
        std::cout << "\nTranscripts:" << std::endl;
        for (int i = 0; i < resp.results_size(); i++) {
            CubicPB::RecognitionResult result = resp.results(i);
            if (!result.is_partial()) {
                std::cout << result.alternatives(0).transcript() << std::endl;
            }
        }
    } catch (CubicException &e) {
        std::cerr << "Cubic error: " << e.what() << std::endl;
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    std::cout << "\nDone." << std::endl;
}
