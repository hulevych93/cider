// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include <iostream>

extern int run_tests(const char* temp_);

extern int run_tests(
    const char* temp,
    const std::function<int(const char* name, std::function<int()>)>& callback);

int main(int argc, char* argv[]) {
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

  std::cout << "Program finished. Press Enter to exit...";
  std::cin.get();

  return 0;
}
