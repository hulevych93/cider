// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include <iostream>

extern void test_value();
extern void test_marshal();

template <typename F>
void record(std::vector<cider::recorder::ScriptRecordSessionPtr>& out, F&& f) {
  auto session = cider::recorder::makeLuaRecordingSession("hjson");
  try {
    f();
  } catch (const std::exception& e) {
    std::cout << e.what();
  }
  out.emplace_back(std::move(session));
}

int main(int argc, char* argv[]) {
  constexpr const char* LibraryName = "hjson";

  cider::Cmd cmd(argc, argv);

  std::vector<cider::recorder::ScriptRecordSessionPtr> sessions;

  // record(sessions, []() { test_value(); });
  // record(sessions, []() { test_marshal(); });

  record(sessions, []() {
    test_value();
    // test_marshal();
  });

  auto pipeline = cider::pipelines::makePipeline(LibraryName, cmd);

  for (const auto& session : sessions) {
    std::vector<cider::recorder::Action> output;
    pipeline.run(session->getInstructions(), output);
  }

  return 0;
}
