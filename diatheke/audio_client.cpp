/*
 * Copyright (2020) Cobalt Speech and Language, Inc.
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

#include <diatheke_client.h>
#include <diatheke_client_error.h>
#include <iostream>

#include "player.h"
#include "recorder.h"

/*
 * Create some aliases to make the code more readable. The gRPC
 * interface can be a bit verbose.
 */
namespace DiathekePB = cobaltspeech::diatheke;
using DiathekeSession = DiathekePB::SessionOutput;

// Some useful variables to define the client configuration
const std::string serverAddress = "localhost:9002";

/*
 * Whether the client connection should be insecure. Must
 * match the server config. Insecure connections are not
 * recommended for production.
 */
const bool insecureConnection = true;

// The model ID to use when initializing a Diatheke session.
const std::string modelID = "1";

// The external process responsible for recording audio.
const std::string recordCmd = "sox -q -d -c 1 -r 16000 -b 16 -L -e signed -t raw -";

// The external process responsible for playing audio.
const std::string playCmd = "sox -q -c 1 -r 48000 -b 16 -L -e signed -t raw - -d";

/*
 * Prompts the user for text input, then returns an updated
 * session based on the user-supplied text.
 */
DiathekeSession waitForInput(Diatheke::Client *client,
                             const DiathekeSession &session,
                             const DiathekePB::WaitForUserAction &inputAction) {
  /*
   * The given input action has a couple of flags to help
   * the app decide when to begin recording audio.
   */
  if (inputAction.immediate()) {
    /*
     * This action is likely waiting for user input in response
     * to a question Diatheke asked, in which case the user should
     * reply immediately. If this flag is false, the app may wait
     * as long as it wants before processing user input (such as
     * waiting for a wake-word below).
     */
  }

  if (inputAction.requires_wake_word()) {
    /*
     * This action requires the wake-word to be spoken before
     * the user input will be accepted. Use a wake-word detector
     * and wait for it to trigger.
     */
  }

  // Create the ASR stream
  Diatheke::ASRStream stream = client->newSessionASRStream(session.token());

  // Start the recorder
  Recorder recorder(recordCmd);
  recorder.start();
  std::cout << "\nRecording..." << std::endl;

  // Record until we get a result
  DiathekePB::ASRResult result =
      Diatheke::ReadASRAudio(stream, &recorder, 8192);
  recorder.stop();

  // Display the result
  std::cout << "\n  ASRResult:" << std::endl;
  std::cout << "    Text: " << result.text() << std::endl;
  std::cout << "    Confidence: " << result.confidence() << std::endl;

  return client->processASRResult(session.token(), result);
}

// Uses TTS to play back the reply as speech.
void handleReply(Diatheke::Client *client,
                 const DiathekePB::ReplyAction &reply) {
  std::cout << "\n  Reply:" << std::endl;
  std::cout << "    Text: " << reply.text() << std::endl;
  std::cout << "    Luna Model: " << reply.luna_model() << std::endl;

  // Create the TTS stream
  Diatheke::TTSStream stream = client->newTTSStream(reply);

  // Start the player
  Player player(playCmd);
  player.start();
  Diatheke::WriteTTSAudio(stream, &player);
  player.stop();
}

/*
 * Executes the task specified by the given command and
 * returns an updated session based on the command result.
 */
DiathekeSession handleCommand(Diatheke::Client *client,
                              const DiathekeSession &session,
                              const DiathekePB::CommandAction &cmd) {
  // Print the command info
  std::cout << std::endl;
  std::cout << "  Command:" << std::endl;
  std::cout << "    ID:" << cmd.id() << std::endl;
  std::cout << "    Input params:" << std::endl;
  auto params = cmd.input_parameters();
  for (auto iter = params.begin(); iter != params.end(); iter++) {
    std::cout << "      " << iter->first << " = " << iter->second << std::endl;
  }

  // Update the session with the command result
  DiathekePB::CommandResult result;
  result.set_id(cmd.id());
  return client->processCommandResult(session.token(), result);
}

/*
 * Executes the actions for the given session and returns
 * an updated session.
 */
DiathekeSession processActions(Diatheke::Client *client,
                               const DiathekeSession &session) {
  // Iterate through each action in the list and determine its type.
  for (auto action : session.action_list()) {
    if (action.has_input()) {
      // The WaitForUserAction will involve a session update.
      return waitForInput(client, session, action.input());
    } else if (action.has_reply()) {
      // Replies do not require a session update.
      handleReply(client, action.reply());
    } else if (action.has_command()) {
      // The CommandAction will involve a session update.
      return handleCommand(client, session, action.command());
    } else {
      throw std::runtime_error("received unknown action type");
    }
  }

  throw std::runtime_error(
      "ran all actions without receiving an updated session");
}

int main(int argc, char *argv[]) {
  try {
    // Create the client
    Diatheke::Client client(serverAddress, insecureConnection);

    // Print the server version info
    auto ver = client.version();
    std::cout << "Server Version" << std::endl;
    std::cout << "  Diatheke: " << ver.diatheke() << std::endl;
    std::cout << "  Chosun (NLU): " << ver.chosun() << std::endl;
    std::cout << "  Cubic (ASR): " << ver.cubic() << std::endl;
    std::cout << "  Luna (TTS): " << ver.luna() << std::endl;

    // Print the list of available models
    auto modelList = client.listModels();
    std::cout << "\nAvailable Models:" << std::endl;
    for (auto &mdl : modelList.models()) {
      std::cout << "  ID: " << mdl.id() << std::endl;
      std::cout << "    Name: " << mdl.name() << std::endl;
      std::cout << "    Language: " << mdl.language() << std::endl;
      std::cout << "    ASR Sample Rate: " << mdl.asr_sample_rate() << std::endl;
      std::cout << "    TTS Sample Rate: " << mdl.tts_sample_rate() << std::endl;
    }

    // Create a session
    auto session = client.createSession(modelID);

    // Loop forever (or until the program is killed)
    while (true) {
      session = processActions(&client, session);
    }

    // Clean up the session.
    client.deleteSession(session.token());
  } catch (const Diatheke::ClientError &e) {
    std::cout << "Diatheke Error: " << e.what() << std::endl;
  } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  }

  return 0;
}
