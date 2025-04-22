// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "recorder/recorder.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include <assert.h>

extern int test_chess_board();
extern int test_primitives();
extern int test_polymorphic_shapes();
extern int test_read_bitmap();
extern int test_rotation();
extern int test_variant_shapes();
extern int test_write_bitmap();

int main(int argc, char* argv[]) {
  auto session = cider::recorder::makeLuaRecordingSession("hjson");

  try {
    const auto start = std::chrono::system_clock::now();

    // test_chess_board();
    // test_primitives();
    test_polymorphic_shapes();
    // test_read_bitmap();
    // test_rotation();
    // test_variant_shapes();
    // test_write_bitmap();

    const auto end = std::chrono::system_clock::now();
    const auto elapsed = end - start;
    std::cout << "Time: "
              << std::chrono::duration_cast<std::chrono::microseconds>(elapsed)
                     .count()
              << " mcs" << std::endl;

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
