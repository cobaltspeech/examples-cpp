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

#include "demo_config.h"

#include <algorithm>
#include <cctype>
#include <cpptoml.h>
#include <stdexcept>


// Converts the Cubic encoding from a string to the enum value.
cobaltspeech::cubic::RecognitionConfig::Encoding
   convertEncoding(std::string encodeStr)
{
    // First make the encoding string lowercase
    std::transform(encodeStr.begin(), encodeStr.end(), encodeStr.begin(),
        [](unsigned char c){ return std::tolower(c); });

    if (encodeStr == "raw_linear16")
    {
        return cobaltspeech::cubic::RecognitionConfig::RAW_LINEAR16;
    }
    if (encodeStr == "wav")
    {
        return cobaltspeech::cubic::RecognitionConfig::WAV;
    }
    if (encodeStr == "mp3")
    {
        return cobaltspeech::cubic::RecognitionConfig::MP3;
    }
    if (encodeStr == "flac")
    {
        return cobaltspeech::cubic::RecognitionConfig::FLAC;
    }
    if (encodeStr == "vox8000")
    {
        return cobaltspeech::cubic::RecognitionConfig::VOX8000;
    }
    if (encodeStr == "ulaw8000")
    {
        return cobaltspeech::cubic::RecognitionConfig::ULAW8000;
    }

    // If we made it this far, we have an unsupported encoding
    throw std::runtime_error("unsupported encoding " + encodeStr);
}

DemoConfig DemoConfig::parseApplicationArgs(int argc, char *argv[])
{
    std::string configFilename;

    if (argc == 1)
    {
        // No arguments were specified. Use a default config filename.
        configFilename = "config.toml";
    }

    // Look for the -config option
    for (int i = 0; i < argc; i++)
    {
        // Check if this is the config arg
        if (strcmp(argv[i], "-config") == 0 || strcmp(argv[i], "--config") == 0)
        {
            // Check that we also have enough args to include the filename
            if (i + 1 < argc)
            {
                configFilename.assign(argv[i + 1]);
                break;
            }
        }
    }

    if (configFilename.empty())
    {
        throw std::runtime_error(
            "missing config file.\n"
            "Please specify the configuration file using the \"-config "
            "<path/to/config.toml>\" option.");
    }

    return parseFile(configFilename);
}

DemoConfig DemoConfig::parseFile(const std::string &filename)
{
    DemoConfig config;

    // Parse the toml file
    std::shared_ptr<cpptoml::table> toml = cpptoml::parse_file(filename);

    // Check for the insecure connection (defaults to false)
    config.mSecure =
        toml->get_qualified_as<bool>("Server.Secure").value_or(true);

    // Check for the server address (required)
    config.mServer = *(toml->get_qualified_as<std::string>("Server.Address"));
    if (config.mServer.empty())
    {
        throw std::runtime_error("required field Server.Address missing");
    }

    // Get the ASR config
    config.mModelID = *(toml->get_qualified_as<std::string>("ASRConfig.ModelID"));
    if(config.mModelID.empty())
    {
        throw std::runtime_error("required field ASRConfig.ModelID missing");
    }

    config.mEncoding = *(toml->get_qualified_as<std::string>("ASRConfig.Encoding"));
    if(config.mEncoding.empty())
    {
        throw std::runtime_error("required field ASRConfig.Encoding missing");
    }

    config.mStream =
        toml->get_qualified_as<bool>("ASRConfig.Streaming").value_or(true);

    // Get the Record command.
    std::string recordApp, recordArgs;
    recordApp = *(toml->get_qualified_as<std::string>("Recording.Application"));
    recordArgs = *(toml->get_qualified_as<std::string>("Recording.Args"));
    config.mRecord = recordApp;
    if (recordArgs.empty() == false)
    {
        config.mRecord += " " + recordArgs;
    }

    return config;
}

DemoConfig::DemoConfig() {}

DemoConfig::~DemoConfig() {}

bool DemoConfig::cubicServerSecure() const { return mSecure; }

const std::string &DemoConfig::cubicServerAddress() const { return mServer; }

const std::string &DemoConfig::modelID() const { return mModelID; }

const std::string &DemoConfig::encoding() const { return mEncoding; }

bool DemoConfig::streaming() const { return mStream; }

const std::string &DemoConfig::recordCmd() const { return mRecord; }

cobaltspeech::cubic::RecognitionConfig DemoConfig::recognitionConfig() const
{
    cobaltspeech::cubic::RecognitionConfig config;

    // Set the Cubic model to use
    config.set_model_id(mModelID);
    
    // Set the encoding to use
    config.set_audio_encoding(convertEncoding(mEncoding));

    // Set the channels to use. For the purposes of the demo, we will
    // assume we only have one, channel 0
    config.add_audio_channels(0);

    return config;
}
