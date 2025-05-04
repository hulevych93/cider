// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include <iostream>

extern int run_tests(const char* temp_);

template <typename F>
void record(std::vector<cider::recorder::ScriptRecordSessionPtr>& out, F&& f) {
  auto session = cider::recorder::makeLuaRecordingSession("pugixml");
  try {
    f();
  } catch (const std::exception& e) {
    std::cout << e.what();
  }
  out.emplace_back(std::move(session));
}

int main(int argc, char* argv[]) {
  constexpr const char* LibraryName = "pugixml";

  cider::Cmd cmd(argc, argv);

  std::vector<cider::recorder::ScriptRecordSessionPtr> sessions;

  record(sessions, [&]() { run_tests(argv[0]); });

  auto pipeline = cider::pipelines::makePipeline(LibraryName, cmd);

  for (const auto& session : sessions) {
    std::vector<cider::recorder::Action> output;
    pipeline.run(session->getInstructions(), output);
  }

  std::cout << "Program finished. Press Enter to exit...";
  std::cin.get();

  return 0;
}
