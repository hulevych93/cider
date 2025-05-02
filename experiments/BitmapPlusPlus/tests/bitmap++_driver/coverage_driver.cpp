// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include <iostream>

extern int test_chess_board();
extern int test_primitives();
extern int test_polymorphic_shapes();
extern int test_read_bitmap();
extern int test_rotation();
extern int test_variant_shapes();
extern int test_write_bitmap();

template <typename F>
void record(std::vector<cider::recorder::ScriptRecordSessionPtr>& out, F&& f) {
  auto session = cider::recorder::makeLuaRecordingSession("bitmap_cplus");
  try {
    f();
  } catch (const std::exception& e) {
    std::cout << e.what();
  }
  out.emplace_back(std::move(session));
}

int main(int argc, char* argv[]) {
  constexpr const char* LibraryName = "bitmap_cplus";

  cider::Cmd cmd(argc, argv);

  std::vector<cider::recorder::ScriptRecordSessionPtr> sessions;

  // record(sessions, []() { test_chess_board(); });
  // record(sessions, []() { test_variant_shapes(); });
  // record(sessions, []() { test_polymorphic_shapes(); });
  // record(sessions, []() { test_read_bitmap(); });
  // record(sessions, []() { test_rotation(); });
  // record(sessions, []() { test_variant_shapes(); });
  // record(sessions, []() { test_write_bitmap(); });

  record(sessions, []() {
    // test_chess_board();
    // test_variant_shapes();
    test_polymorphic_shapes();
    // test_read_bitmap();
    // test_rotation();
    // test_variant_shapes();
    // test_write_bitmap();
  });

  auto pipeline = cider::pipelines::makePipeline(LibraryName, cmd);

  for (const auto& session : sessions) {
    std::vector<cider::recorder::Action> output;
    pipeline.run(session->getInstructions(), output);
  }

  return 0;
}
