// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "experiments/pipelines/metapipeline.h"
#include "experiments/pipelines/q-learning-pipeline.h"

#include <iostream>

extern int test_chess_board();
extern int test_primitives();
extern int test_polymorphic_shapes();
extern int test_read_bitmap();
extern int test_rotation();
extern int test_variant_shapes();
extern int test_write_bitmap();

template<typename F>
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
  cider::coverage::Cmd cmd(argc, argv);

  std::vector<cider::recorder::ScriptRecordSessionPtr> sessions;

  //record(sessions, []() { test_chess_board(); });
  record(sessions, []() { test_primitives(); });
  //record(sessions, []() { test_polymorphic_shapes(); });
  //record(sessions, []() { test_read_bitmap(); });
  //record(sessions, []() { test_rotation(); });
  //record(sessions, []() { test_variant_shapes(); });
  //record(sessions, []() { test_write_bitmap(); });

  return cider::pipelines::qlearningPipeline("bitmap_cplus", cmd,
                                             std::move(sessions));
}
