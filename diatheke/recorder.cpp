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

#include "recorder.h"

#include <unistd.h>

Recorder::Recorder(const std::string &recordCmd)
    : mCmd(recordCmd), mStdout(nullptr) {}

Recorder::~Recorder() {
  // Make sure the recorder is stopped
  this->stop();
}

void Recorder::start() {
  // Ignore if the recorder is already running
  if (mStdout)
    return;

  // Start the external process
  mStdout = popen(mCmd.c_str(), "r");
}

size_t Recorder::readAudio(char *buffer, size_t buffSize) {
  // Throw an error if the recorder is not running.
  if (mStdout == nullptr) {
    throw std::runtime_error("can't read audio - recorder not started.");
  }

  return fread(buffer, 1, buffSize, mStdout);
}

void Recorder::stop() {
  // Ignore if the recorder is already stopped
  if (mStdout == nullptr) {
    return;
  }

  // Close the stdout stream, which should also close the application.
  pclose(mStdout);
  mStdout = nullptr;
}
