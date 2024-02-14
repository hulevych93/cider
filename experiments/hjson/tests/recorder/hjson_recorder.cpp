// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "recorder/recorder.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include <assert.h>

extern void test_value();
extern void test_marshal();

int main(int argc, char* argv[]) {
  auto session = cider::recorder::makeLuaRecordingSession("hjson");

  try {
    test_marshal();
  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  if (argc == 2) {
    std::cout << "script path: " << argv[1];
    try {
      const auto script = session->getScript(session->getInstructionsCount());

      std::ofstream output_file(argv[1]);
      output_file << script;
    } catch (const std::exception& e) {
      std::cerr << e.what();
      return 1;
    }
  }

  return 0;
}
