// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include <iostream>

extern void run_tests(
    const std::function<void(const char* name, std::function<void()>)>&
        callback);

int main(int argc, char* argv[]) {
  constexpr const char* LibraryName = "hjson";

  cider::Cmd cmd(argc, argv);

  std::vector<cider::recorder::ScriptRecordSessionPtr> sessions;

  if (0) {
    const auto callback = [&](const char* testName,
                              const std::function<void()>& f) {
      cider::recorder::recordScript(LibraryName, testName, sessions, f);
    };

    run_tests(callback);
  } else {
    std::vector<std::function<void()>> tests;

    const auto callback = [&tests](const char*,
                                   const std::function<void()>& f) {
      tests.emplace_back(f);
    };

    run_tests(callback);

    cider::recorder::recordScript(LibraryName, "hjson", sessions, [&]() {
      for (const auto& test : tests) {
        test();
      }
    });
  }

  auto pipeline = cider::pipelines::makePipeline(LibraryName, cmd);
  pipeline.runOneByOne(sessions);

  std::cout << "Program finished. Press Enter to exit...";
  std::cin.get();

  return 0;
}
