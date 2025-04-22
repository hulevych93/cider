// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "experiments/pipelines/metapipeline.h"

extern int test_chess_board();
extern int test_primitives();
extern int test_polymorphic_shapes();
extern int test_read_bitmap();
extern int test_rotation();
extern int test_variant_shapes();
extern int test_write_bitmap();

int main(int argc, char* argv[]) {
  cider::coverage::Cmd cmd(argc, argv);

  auto session = cider::recorder::makeLuaRecordingSession("bitmap_cplus");

  try {
    // test_chess_board();
    //test_primitives();
    // test_polymorphic_shapes();
    // test_read_bitmap();
    // test_rotation();
    // test_variant_shapes();
    // test_write_bitmap();
  } catch (const std::exception& e) {
    std::cout << e.what();
  }

  return cider::pipelines::metaPipeline(
      "bitmap_cplus", cmd, std::move(session),
      cider::metasearch::MutationStrategy::LevyFlight);
}
