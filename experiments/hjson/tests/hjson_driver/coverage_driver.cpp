// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipelines/pipeline.h"

#include <iostream>

extern void test_value();
extern void test_marshal();

static void testAll() {
  test_value();
  test_marshal();
}

int main(int argc, char* argv[]) {
  constexpr const char* LibraryName = "hjson";

  cider::Cmd cmd(argc, argv);

  std::vector<cider::recorder::ScriptRecordSessionPtr> sessions;

  if (0) {
    RECORD_TEST_SCRIPT(LibraryName, test_value, sessions);
    RECORD_TEST_SCRIPT(LibraryName, test_marshal, sessions);
  } else {
    RECORD_TEST_SCRIPT(LibraryName, testAll, sessions);
  }

  std::sort(sessions.begin(), sessions.end(),
            [](const cider::recorder::ScriptRecordSessionPtr& a,
               const cider::recorder::ScriptRecordSessionPtr& b) {
              return a->getInstructions().size() > b->getInstructions().size();
            });

  auto pipeline = cider::pipelines::makePipeline(LibraryName, cmd);
  pipeline.run(sessions);

  std::cout << "Program finished. Press Enter to exit...";
  std::cin.get();

  return 0;
}
