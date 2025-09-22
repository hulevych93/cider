// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include <tlog.h>

extern void run_tests(
    const std::function<void(const char* name, std::function<void()>)>&
        callback);

int main(int argc, char* argv[]) {
  constexpr const char* LibraryName = "argh";

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

    cider::recorder::recordScript(LibraryName, "testAll", sessions, [&]() {
      for (const auto& test : tests) {
        test();
      }
    });
  }

  auto pipeline = cider::pipelines::makePipeline(LibraryName, cmd);
  pipeline.run(sessions);

  return 0;
}
