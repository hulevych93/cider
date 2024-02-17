// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "recorder/recorder.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include <assert.h>

#include "experiments/pugixml/tests/pugixml_tests/test.hpp"

int main(int argc, char** argv) {
  std::string temp = argv[0];
  int result = 0;

  auto session = cider::recorder::makeLuaRecordingSession("pugixml");

  try {
    run_tests(temp.c_str());

    if (argc == 2) {
      std::cout << "script path: " << argv[1] << std::endl;
      try {
        const auto script = session->getScript(session->getInstructionsCount());

        std::ofstream output_file(argv[1]);
        output_file << script;
      } catch (const std::exception& e) {
        std::cerr << e.what();
        return 1;
      }
    }
  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  return result;
}
