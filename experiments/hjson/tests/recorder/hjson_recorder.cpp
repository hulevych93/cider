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
  assert(argc == 2);
  std::cout << "working directory: " << argv[1];
  std::filesystem::create_directories(argv[1]);

  auto session = cider::recorder::makeLuaRecordingSession("hjson");

  try {
    test_value();
    test_marshal();
  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  const auto count = session->getInstructionsCount();
  for (auto i = 1U; i < count; ++i) {
    try {
      const auto script = session->getScript(i);

      auto outPath = std::string{argv[1]};
      std::filesystem::create_directories(outPath);

      std::ofstream output_file(outPath + "/script_" + std::to_string(i) +
                                ".lua");
      output_file << script;
    } catch (const std::exception& e) {
      std::cerr << e.what();
      return 1;
    }
  }

  return 0;
}
