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

/*
 * Prompts the user for text input, then returns an updated
 * session based on the user-supplied text.
 */
DiathekeSession waitForInput(Diatheke::Client *client,
                             const DiathekeSession &session) {
  std::cout << std::endl << "\nDiatheke> " << std::flush;

  // Wait for user input
  std::string text;
  std::getline(std::cin, text);

  return client->processText(session.token(), text);
}

// Prints the text of the given reply to stdout.
void handleReply(const DiathekePB::ReplyAction &reply) {
  std::cout << "\n  Reply:" << reply.text() << std::endl;
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
      return waitForInput(client, session);
    } else if (action.has_reply()) {
      // Replies do not require a session update.
      handleReply(action.reply());
    } else if (action.has_command()) {
      // The CommandAction will involve a session update.
      return handleCommand(client, session, action.command());
    } else {
      throw std::runtime_error("received unknown action type");
    }
  }

  throw std::runtime_error("action list ended without session update");
}

int main(int argc, char *argv[]) {
  // Create the client
  Diatheke::Client client(serverAddress, insecureConnection);

  // Request the server version info
  auto ver = client.version();
  std::cout << "Server Version" << std::endl;
  std::cout << "  Diatheke: " << ver.diatheke() << std::endl;
  std::cout << "  Chosun (NLU): " << ver.chosun() << std::endl;
  std::cout << "  Cubic (ASR): " << ver.cubic() << std::endl;
  std::cout << "  Luna (TTS): " << ver.luna() << std::endl;

  // Request the list of available models
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
  std::string modelID = "1";
  auto session = client.createSession(modelID);

  try {
    // Loop forever (or until the program is killed)
    while (true) {
      session = processActions(&client, session);
    }
  } catch (const Diatheke::ClientError &e) {
    std::cout << "Diatheke Error: " << e.what() << std::endl;
  } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  }

  // Clean up the session.
  client.deleteSession(session.token());
  return 0;
}
