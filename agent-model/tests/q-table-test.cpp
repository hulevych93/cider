// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "agent-model/qtable-agent.h"

using namespace cider::recorder;
using namespace cider::agent_model;

Actions cut_suffix(const Actions& actions, size_t suffixLen) {
  if (actions.size() <= suffixLen)
    return actions;
  return Actions(actions.end() - suffixLen, actions.end());
}

class AgentModelTestSuite : public testing::Test {};

TEST_F(AgentModelTestSuite, test) {
  const size_t k = 3;

  auto f1 = makeAction("f1", Nil{}, 1);
  auto f2 = makeAction("f2", Nil{}, 2);
  auto f3 = makeAction("f3", Nil{}, 3);
  auto f4 = makeAction("f4", Nil{}, 4);
  auto f5 = makeAction("f3", Nil{}, 234);

  Actions script1 = {f1, f2, f3};
  Actions script2 = {f1, f2, f3, f4};
  Actions script3 = {f1, f2, f1, f2, f5};

  QTable qtable;

  qtable[cut_suffix(script1, k)] = {};
  qtable[cut_suffix(script2, k)] = {};

  const auto check = [&](const Actions& script) {
    auto it = qtable.find(cut_suffix(script, k));
    EXPECT_TRUE(it != qtable.end());
  };

  check(script1);
  check(script2);
  check(script3);
}

TEST_F(AgentModelTestSuite, SameFunctionNameDifferentBoolInt) {
  auto a1 = makeAction("f", Nil{}, true);
  auto a2 = makeAction("f", Nil{}, false);
  auto a3 = makeAction("f", Nil{}, 123);

  Actions script1 = {a1, a2, a3};

  QTable qtable;
  qtable[cut_suffix(script1, 3)] = {};

  EXPECT_NE(qtable.find(cut_suffix(script1, 3)), qtable.end());
}

TEST_F(AgentModelTestSuite, SameFunctionNameDifferentString) {
  auto a1 = makeAction("f", Nil{}, "a");
  auto a2 = makeAction("f", Nil{}, "b");
  auto a3 = makeAction("f", Nil{}, "c");

  Actions script1 = {a1, a2, a3};

  QTable qtable;
  qtable[cut_suffix(script1, 3)] = {};

  EXPECT_NE(qtable.find(cut_suffix(script1, 3)), qtable.end());
}

TEST_F(AgentModelTestSuite, SameFunctionNameDifferentDouble) {
  auto a1 = makeAction("f", Nil{}, 1.1);
  auto a2 = makeAction("f", Nil{}, 2.2);
  auto a3 = makeAction("f", Nil{}, 3.3);

  Actions script1 = {a1, a2, a3};

  QTable qtable;
  qtable[cut_suffix(script1, 3)] = {};

  EXPECT_NE(qtable.find(cut_suffix(script1, 3)), qtable.end());
}

TEST_F(AgentModelTestSuite, SameFunctionNameUserDataSimulated) {
  auto a1 = makeAction("f", Nil{}, Nil{});
  auto a2 = makeAction("f", Nil{}, Nil{});
  auto a3 = makeAction("f", Nil{}, Nil{});

  Actions script1 = {a1, a2, a3};

  QTable qtable;
  qtable[cut_suffix(script1, 3)] = {};

  EXPECT_NE(qtable.find(cut_suffix(script1, 3)), qtable.end());
}

TEST_F(AgentModelTestSuite, DifferentParamCountShouldFail) {
  auto a1 = makeAction("f", Nil{}, 1);
  auto a2 = makeAction("f", Nil{}, 2);
  auto a3 = makeAction("f", Nil{}, 3);

  auto a4 = makeAction("f", Nil{}, 3, 5);

  Actions script1 = {a1, a2, a3};
  Actions script2 = {a2, a3, a4};

  QTable qtable;
  qtable[cut_suffix(script1, 3)] = {};

  EXPECT_EQ(qtable.find(cut_suffix(script2, 3)), qtable.end());
}
