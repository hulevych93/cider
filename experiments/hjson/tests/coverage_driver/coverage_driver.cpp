// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "experiments/pipelines/metapipeline.h"

#include <iostream>

extern void test_value();
extern void test_marshal();

int main(int argc, char* argv[]) {
  cider::Cmd cmd(argc, argv);

  auto session = cider::recorder::makeLuaRecordingSession("hjson");

  try {
    test_value();
    // test_marshal();
  } catch (const std::exception& e) {
    std::cout << e.what();
  }

  return cider::pipelines::metaCfgPipeline("hjson", cmd, std::move(session));
}
