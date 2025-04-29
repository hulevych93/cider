// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/metapipeline.h"
#include "pipelines/q-learning-pipeline.h"

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
  cider::Cmd cmd(argc, argv);

  std::vector<cider::recorder::ScriptRecordSessionPtr> sessions;

  record(sessions, [&]() { run_tests(argv[0]); });

  if (cmd.pipelineType == cider::PipelineType::HarmonySearch ||
      cmd.pipelineType == cider::PipelineType::CackooSearch) {
    for (const auto& session : sessions) {
      cider::pipelines::metaCfgPipeline("pugixml", cmd, std::move(session));
    }
    return 0;

  } else {
    return cider::pipelines::qlearningCfgPipeline("pugixml", cmd,
                                                  std::move(sessions));
  }
}
