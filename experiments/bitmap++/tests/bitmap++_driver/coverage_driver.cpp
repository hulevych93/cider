// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include <pybind11/embed.h>
namespace py = pybind11;

#include <tlog.h>

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

  const auto getTCs = []() {
    std::vector<cider::recorder::ScriptRecordSessionPtr> sessions;
    RECORD_TEST_SCRIPT(LibraryName, test_chess_board, sessions);
    RECORD_TEST_SCRIPT(LibraryName, test_variant_shapes, sessions);
    RECORD_TEST_SCRIPT(LibraryName, test_polymorphic_shapes, sessions);
    RECORD_TEST_SCRIPT(LibraryName, test_read_bitmap, sessions);
    RECORD_TEST_SCRIPT(LibraryName, test_rotation, sessions);
    RECORD_TEST_SCRIPT(LibraryName, test_primitives, sessions);
    RECORD_TEST_SCRIPT(LibraryName, test_write_bitmap, sessions);
    return sessions;
  };

  const auto getTS = []() {
    std::vector<cider::recorder::ScriptRecordSessionPtr> sessions;
    RECORD_TEST_SCRIPT(LibraryName, bitmap_cplusplus, sessions);
    return sessions;
  };

  auto pipeline = cider::pipelines::makePipeline(LibraryName, cmd);
  pipeline.run(getTS, getTCs);

  if (pipeline.newResuls()) {
    tlog_info << "Save results? (y/n): ";
    char decision;
    if (!cider::pipelines::isDebuggerAttached()) {
      std::cin >> decision;
    } else {
      decision = 'Y';
    }
    if (decision == 'y' || decision == 'Y') {
      pipeline.save();
      tlog_info << "Saved.\n";
    }
  }

  return 0;
}
