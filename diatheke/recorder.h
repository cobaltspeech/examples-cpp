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

#ifndef RECORDER_H
#define RECORDER_H

#include <cstdio>
#include <diatheke_asr_stream.h>
#include <string>

/*
 * Recorder receives audio data from an external application
 * and forwards it to Diatheke for ASR processing. The external
 * application can be anything as long as it supports the encoding
 * required by the underlying ASR model, and can stream audio data
 * to stdout.
 */
class Recorder : public Diatheke::AudioReader {
public:
  /*
   * Create a new recorder instance that will launch the given external
   * application (recordCmd).
   */
  Recorder(const std::string &recordCmd);
  ~Recorder();

  // Start recording audio.
  void start();

  /*
   * Re-implemented from Diatheke::AudioReader. Reads audio
   * data from the recorder application, and stores it in the given
   * buffer, returning the number of bytes read.
   */
  size_t readAudio(char *buffer, size_t buffSize) override;

  // Stop recording audio, and return the recorded data.
  void stop();

private:
  std::string mCmd;
  FILE *mStdout;
};

#endif // RECORDER_H
