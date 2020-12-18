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

#ifndef PLAYER_H
#define PLAYER_H

#include <cstdio>
#include <diatheke_tts_stream.h>
#include <string>
#include <vector>

/*
 * Player receives audio data from Diatheke and forwards it to an
 * external application for playback over a device. The external
 * application may be anything as long as it supports the encoding
 * required by the underlying TTS model, and can receive audio data
 * from stdin.
 */
class Player : public Diatheke::AudioWriter {
public:
  /*
   * Constructs a new player object using the given command, which
   * will be used to launch the external playback application.
   */
  Player(const std::string &playCmd);

  /*
   * Destroys the player object. If the player has not been already
   * been stopped, this will also stop the external application.
   */
  ~Player();

  // Start the external playback application.
  void start();

  // Stop the external playback application.
  void stop();

  /*
   * Re-implements the AudioWriter interface. Writes the given
   * audio data to the playback application.
   */
  size_t writeAudio(const char *audio, size_t sizeInBytes) override;

private:
  std::string mCmd;
  FILE *mStdin;
};

#endif // PLAYER_H
