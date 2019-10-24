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

#ifndef DEMO_CONFIG_H
#define DEMO_CONFIG_H

#include "cubic.pb.h"

#include <string>

class DemoConfig
{
public:
    /*
     * Parse the command-line arguments given to application at startup
     * and return the configuration based on the args. It is expecting
     * "-config <path/to/config.toml>" to be included in the arguments and
     * throws an exception if they are not (or if there are errors parsing).
     */
    static DemoConfig parseApplicationArgs(int argc, char *argv[]);

    /*
     * Parse the given configuration file and return a DemoConfig object.
     * Throws an exception if errors are encountered.
     */
    static DemoConfig parseFile(const std::string &filename);

    DemoConfig();
    ~DemoConfig();

    // Returns true if a secure connection should be made with Cubic server.
    bool cubicServerSecure() const;

    // Returns the url of the Cubic server (with port number)
    const std::string &cubicServerAddress() const;

    // Returns the ID of the Cubic model to use
    const std::string &modelID() const;

    // Returns the expected audio encoding for recorded audio data.
    const std::string &encoding() const;

    // Returns true if the streaming recognition API should be used
    bool streaming() const;

    // Returns the record command (with args)
    const std::string &recordCmd() const;

    /*
     * Returns a RecognitionConfig suitable for use in Cubic requests,
     * based on the config file parameters.
     */
    cobaltspeech::cubic::RecognitionConfig recognitionConfig() const;

private:
    bool mSecure;
    std::string mServer;
    std::string mModelID;
    std::string mEncoding;
    bool mStream;
    std::string mRecord;
};

#endif // DEMO_CONFIG_H
