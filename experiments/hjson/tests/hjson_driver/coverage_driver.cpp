// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include <pybind11/embed.h>
namespace py = pybind11;

#include <iostream>

extern void run_tests(
    const std::function<void(const char* name, std::function<void()>)>&
        callback);

int main(int argc, char* argv[]) {
  py::scoped_interpreter guard{};

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

  if (pipeline.newResuls()) {
    std::cout << "Save results? (y/n): ";
    char decision;
    std::cin >> decision;
    if (decision == 'y' || decision == 'Y') {
      pipeline.save();
      std::cout << "Saved.\n";
    }
  }

  return 0;
}
