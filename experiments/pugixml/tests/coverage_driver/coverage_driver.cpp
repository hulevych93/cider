// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "experiments/pipelines/metapipeline.h"

#include <iostream>

extern int run_tests(const char* temp_);

int main(int argc, char* argv[]) {
  cider::Cmd cmd(argc, argv);

  auto session = cider::recorder::makeLuaRecordingSession("pugixml");

  try {
    run_tests(argv[0]);
  } catch (const std::exception& e) {
    std::cout << e.what();
  }

  return cider::pipelines::metaCfgPipeline("pugixml", cmd, std::move(session));
}
