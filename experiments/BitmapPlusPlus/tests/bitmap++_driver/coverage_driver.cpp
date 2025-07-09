// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include <pybind11/embed.h>
namespace py = pybind11;

#include <iostream>

extern int test_chess_board();
extern int test_primitives();
extern int test_polymorphic_shapes();
extern int test_read_bitmap();
extern int test_rotation();
extern int test_variant_shapes();
extern int test_write_bitmap();

static void bitmap_cplusplus() {
  test_chess_board();
  test_variant_shapes();
  test_polymorphic_shapes();
  test_read_bitmap();
  test_rotation();
  test_primitives();
  test_write_bitmap();
}

int main(int argc, char* argv[]) {
  py::scoped_interpreter guard{};

  constexpr const char* LibraryName = "bitmap_cplusplus";

  cider::Cmd cmd(argc, argv);

  std::vector<cider::recorder::ScriptRecordSessionPtr> sessions;

  if (0) {
    RECORD_TEST_SCRIPT(LibraryName, test_chess_board, sessions);
    RECORD_TEST_SCRIPT(LibraryName, test_variant_shapes, sessions);
    RECORD_TEST_SCRIPT(LibraryName, test_polymorphic_shapes, sessions);
    RECORD_TEST_SCRIPT(LibraryName, test_read_bitmap, sessions);
    RECORD_TEST_SCRIPT(LibraryName, test_rotation, sessions);
    RECORD_TEST_SCRIPT(LibraryName, test_primitives, sessions);
    RECORD_TEST_SCRIPT(LibraryName, test_write_bitmap, sessions);
  } else {
    RECORD_TEST_SCRIPT(LibraryName, bitmap_cplusplus, sessions);
  }

  std::sort(sessions.begin(), sessions.end(),
            [](const cider::recorder::ScriptRecordSessionPtr& a,
               const cider::recorder::ScriptRecordSessionPtr& b) {
              return a->getInstructions().size() > b->getInstructions().size();
            });

  auto pipeline = cider::pipelines::makePipeline(LibraryName, cmd);
  pipeline.runOneByOne(sessions);

  std::cout << "Program finished. Press Enter to exit...";
  std::cin.get();

  return 0;
}
