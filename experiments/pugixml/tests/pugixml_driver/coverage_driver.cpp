// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include <pybind11/embed.h>
namespace py = pybind11;

#include <iostream>

extern int run_tests(const char* temp_);

extern int run_tests(
    const char* temp,
    const std::function<int(const char* name, std::function<int()>)>& callback);

int main(int argc, char* argv[]) {
  py::scoped_interpreter guard{};

  constexpr const char* LibraryName = "pugixml";

  cider::Cmd cmd(argc, argv);

  std::vector<cider::recorder::ScriptRecordSessionPtr> sessions;

  if (0) {
    const auto callback = [&](const char* testName,
                              const std::function<int()>& f) -> int {
      return cider::recorder::recordScriptWithResult(LibraryName, testName,
                                                     sessions, f);
    };

    run_tests(argv[0], callback);
  } else {
    cider::recorder::recordScript(LibraryName, "pugixml", sessions,
                                  [&]() { run_tests(argv[0]); });
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
