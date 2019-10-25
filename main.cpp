/*
 * Copyright (2019) Cobalt Speech and Language, Inc.
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
#include "demo_config.h"
#include "recorder.h"
#include "timer.h"

#include <atomic>
#include <iostream>
#include <regex>
#include <string>
#include <thread>


// Wait for the Enter key to be pressed
void waitForEnter()
{
    // This is a somewhat simplistic way to detect if the enter key was
    // pressed, but it is sufficient for demo purposes.
    std::string line_input;
    std::getline(std::cin, line_input);
}

// Get the transcript from the response
std::string extractTranscript(const cobaltspeech::cubic::RecognitionResponse &response)
{
    std::string transcript;
    for(int i = 0; i < response.results_size(); i++)
    {
        cobaltspeech::cubic::RecognitionResult result = response.results(i);

        // Ignore partial results
        if(result.is_partial())
            continue;

        // If we don't have any alternatives, there isn't much for us
        // to do right now.
        if(result.alternatives_size() == 0)
            continue;

        // Just use the first alternative for now.
        transcript += result.alternatives(0).transcript() + " ";
    }

    std::regex trim("^\\s+|\\s+$");
    transcript = std::regex_replace(transcript, trim, "");
    return transcript;
}

/*
 * Run batch speech recognition, where we submit all the recorded audio at
 * once and get a transcript back.
 */
void recognize(CubicClient *client, const DemoConfig &config)
{
    std::cout << "Press ENTER to start recording (Ctrl+C to exit)" << std::endl;
    waitForEnter();
    std::cout << "Running ASR (press ENTER to stop)..." << std::endl;

    Timer totalTimer;

    /*
     * Setup a thread to read audio data from the recorder and store
     * it in a string (think of the string as an array of bytes). We
     * run it on a separate thread so that we can wait for the user
     * to press the Enter key on the main thread to stop it.
     */
    std::atomic_bool stopAudio(false);
    std::string audioBuffer;
    auto recordAudio = [&stopAudio, &audioBuffer, &config]()
    {
        // Start recording
        Recorder rec(config.recordCmd());
        rec.start();

        // Collect data
        while(!stopAudio)
            audioBuffer += rec.readAudio();

        // Stop recording
        rec.stop();
    };

    // Start the thread
    Timer recordTimer;
    std::thread audioThread(recordAudio);

    // On the main thread, wait for a keypress to end recording
    waitForEnter();
    stopAudio = true;
    audioThread.join();
    double recordDuration = recordTimer.elapsed();

    // Send the data to Cubic for the transcription
    cobaltspeech::cubic::RecognitionConfig rConf = config.recognitionConfig();
    Timer cubicTimer;
    cobaltspeech::cubic::RecognitionResponse response =
        client->recognize(rConf, audioBuffer.c_str(), audioBuffer.length());
    double cubicDuration = cubicTimer.elapsed();

    // Print the result
    std::string result = extractTranscript(response);
    if(result.empty())
    {
        std::cout << "Unrecognized result: " << response.Utf8DebugString();
    }
    else
    {
        std::cout << "Transcription: " << result;
    }
    std::cout << std::endl << std::endl;

    // Print out statistics
    double totalDuration = totalTimer.elapsed();
    std::cout << "  ASR statistics" << std::endl
              << "  --------------" << std::endl
              << "  record time: " << recordDuration << "s" << std::endl
              << "  cubic time: " << cubicDuration << "s" << std::endl
              << "    real time factor (record/cubic): "
              << recordDuration/cubicDuration << std::endl
              << "  total ASR time: " << totalDuration << "s" << std::endl
              << std::endl;
}

/*
 * Run streaming speech recognition, where results may come back while audio
 * is still being pushed to the server. This is the recommended approach for
 * Cubic, particularly if the input audio stream is long.
 */
void streamingRecognize(CubicClient *client, const DemoConfig &config)
{
    std::cout << "Press ENTER to start recording (Ctrl+C to exit)" << std::endl;
    waitForEnter();
    std::cout << "Running ASR (press ENTER to stop)..." << std::endl;

    Timer totalTimer;

    // Create the streaming recognizer
    cobaltspeech::cubic::RecognitionConfig rConf = config.recognitionConfig();
    CubicRecognizerStream stream = client->streamingRecognize(rConf);

    // Setup a thread to read audio and push it to Cubic.
    std::atomic_bool stopAudio(false);
    auto recordAudio = [&stopAudio, &stream, &config]()
    {
        // Start recording
        Recorder rec(config.recordCmd());
        rec.start();

        // Collect data and push to Cubic
        while(!stopAudio)
        {
            std::string audioBuffer = rec.readAudio();
            stream.pushAudio(audioBuffer.c_str(), audioBuffer.length());
        }

        // Let Cubic know that no more audio will be coming and stop
        // the recorder.
        stream.audioFinished();
        rec.stop();
    };

    // Setup a thread to get results from Cubic
    struct
    {
        std::vector<std::string> transcripts;
        double timeToFirst;
        double timeToLast;
    } allResultData;

    auto receiveResults = [&stream, &allResultData]()
    {
        Timer resultTimer;
        bool firstResponse(true);

        // Wait for a response from the server. Continue until the
        // stream is finished.
        cobaltspeech::cubic::RecognitionResponse response;
        while(stream.receiveResults(&response))
        {
            if(firstResponse)
            {
                allResultData.timeToFirst = resultTimer.elapsed();
                firstResponse = false;
            }

            std::string transcript = extractTranscript(response);
            allResultData.transcripts.push_back(transcript);

            // Print the transcription as it comes.
            if(transcript.empty())
                std::cout << "Unrecognized result: " <<
                             response.Utf8DebugString() << std::endl;
            else
                std::cout << "Transcription: " << transcript << std::endl;
        }

        allResultData.timeToLast = resultTimer.elapsed();
    };

    // Start the threads
    std::thread audioThread(recordAudio);
    std::thread resultsThread(receiveResults);

    // On the main thread, wait for another key press to stop recording.
    waitForEnter();
    stopAudio = true;
    audioThread.join();
    double recordDuration = totalTimer.elapsed();

    resultsThread.join();
    stream.close();

    // Print statistics
    std::cout << std::endl;
    std::cout << "  Streaming ASR statistics" << std::endl
              << "  ------------------------" << std::endl
              << "  record time: " << recordDuration << "s" << std::endl
              << "  time to first response: " << allResultData.timeToFirst << "s" << std::endl
              << "  time to last response: " << allResultData.timeToLast << "s" << std::endl
              << "  total ASR time: " << totalTimer.elapsed() << "s" << std::endl << std::endl;
}


int main(int argc, char *argv[])
{
    // Parse the config file
    DemoConfig config;
    try
    {
        config = DemoConfig::parseApplicationArgs(argc, argv);

        // The following is required for this demo
        if (config.recordCmd().empty())
        {
            std::cerr << "missing Recording Application in the config file"
                      << std::endl;
            return 1;
        }
    }
    catch (const std::exception &err)
    {
        std::cerr << "error parsing config file: " << err.what() << std::endl;
        return 1;
    }

    // Create the client
    CubicClient client(config.cubicServerAddress(),
                       config.cubicServerSecure());

    // Display the diatheke version
    std::cout << "Cubic version: " << client.cubicVersion() << std::endl;
    std::cout << "Server version: " << client.serverVersion() << std::endl;
    std::cout << "Connected to " << config.cubicServerAddress() << std::endl;
    std::cout << std::endl;

    // Display available models
    std::vector<CubicModel> models = client.listModels();
    std::cout << "Cubic Models:" << std::endl;
    for(const CubicModel &m : models)
    {
        std::cout << "  Model ID: " << m.id() <<
                     "  Name: " << m.name() <<
                     "  Sample rate: " << m.sampleRate() << " Hz" << std::endl;
    }
    std::cout << std::endl;

    // Main loop
    while (true)
    {
        try
        {
            if (config.streaming())
            {
                // Run streaming speech recognition
                streamingRecognize(&client, config);
            }
            else
            {
                // Run batch recognition
                recognize(&client, config);
            }
        }
        catch(CubicException &e)
        {
            std::cerr << "Cubic error: " << e.what() << std::endl;
            continue;
        }
        catch(std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            break;
        }
    }

    // Cleanup
    std::cout << std::endl << "Exiting..." << std::endl;

    return 0;
}
