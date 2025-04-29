// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/metapipeline.h"
#include "pipelines/q-learning-pipeline.h"

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
  cider::Cmd cmd(argc, argv);

  std::vector<cider::recorder::ScriptRecordSessionPtr> sessions;

  // record(sessions, []() { test_value(); });
  // record(sessions, []() { test_marshal(); });

  record(sessions, []() {
    test_value();
    // test_marshal();
  });

  if (cmd.pipelineType == cider::PipelineType::HarmonySearch ||
      cmd.pipelineType == cider::PipelineType::CackooSearch) {
    for (const auto& session : sessions) {
      cider::pipelines::metaCfgPipeline("hjson", cmd, std::move(session));
    }
    return 0;

  } else {
    return cider::pipelines::qlearningCfgPipeline("hjson", cmd,
                                                  std::move(sessions));
  }
}
