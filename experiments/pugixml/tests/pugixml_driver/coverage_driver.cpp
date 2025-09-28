// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include <pybind11/embed.h>
namespace py = pybind11;

#include <tlog.h>

extern int run_tests(const char* temp_);

extern int run_tests(
    const char* temp,
    const std::function<int(const char* name, std::function<int()>)>& callback);

int main(int argc, char* argv[]) {
  py::scoped_interpreter guard{};

  constexpr const char* LibraryName = "pugixml";

  cider::Cmd cmd(argc, argv);

  const auto getTCs = [argv]() {
    std::vector<cider::recorder::ScriptRecordSessionPtr> sessions;
    const auto callback = [&](const char* testName,
                              const std::function<int()>& f) -> int {
      return cider::recorder::recordScriptWithResult(LibraryName, testName,
                                                     sessions, f);
    };

    run_tests(argv[0], callback);
    return sessions;
  };

  const auto getTS = [argv]() {
    std::vector<cider::recorder::ScriptRecordSessionPtr> sessions;
    cider::recorder::recordScript(LibraryName, "pugixml", sessions,
                                  [&]() { run_tests(argv[0]); });
    return sessions;
  };

  auto pipeline = cider::pipelines::makePipeline(LibraryName, cmd);
  pipeline.run(getTS, getTCs);

  if (pipeline.newResuls()) {
    tlog_info << "Save results? (y/n): ";
    char decision;
    if (!cider::isDebuggerAttached()) {
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
