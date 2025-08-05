// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "agent-model/qtable-agent.h"

using namespace cider::recorder;
using namespace cider::agent_model;

class AgentModelValuesTestSuite : public testing::Test {};

TEST(AgentModelValuesTestSuite, FunctionFullyEqualMatch) {
  auto a1 = makeAction("f", Nil{}, 1);
  auto a2 = makeAction("f", Nil{}, 1);

  QValues q;
  q[a1] = 3.14;

  EXPECT_NE(q.find(a2), q.end());
}

TEST(AgentModelValuesTestSuite, FunctionDifferentParamFails) {
  auto a1 = makeAction("f", Nil{}, 1);
  auto a2 = makeAction("f", Nil{}, 999);

  QValues q;
  q[a1] = 3.14;

  EXPECT_EQ(q.find(a2), q.end());
}

TEST(FuzzyQValuesTest, ClassMethodDifferentObjectAddressMatch) {
  auto a1 = makeAction((void*)0x1111, "f", Nil{}, 999);
  auto a2 = makeAction((void*)0x2222, "f", Nil{}, 999);

  QValues q;
  q[a1] = 1.23;

  auto it = q.find(a2);
  EXPECT_NE(it, q.end());
  if (it != q.end())
    EXPECT_DOUBLE_EQ(it->second, 1.23);
}

TEST(FuzzyQValuesTest, ClassMethodDifferentParamFails) {
  auto a1 = makeAction((void*)0x1111, "f", Nil{}, 42);
  auto a2 = makeAction((void*)0x1111, "f", Nil{}, 777);

  QValues q;
  q[a1] = 9.99;

  EXPECT_EQ(q.find(a2), q.end());
}

TEST(FuzzyQValuesTest, ClassMethodDifferentNameFails) {
  auto a1 = makeAction((void*)0x1111, "foo", Nil{}, 42);
  auto a2 = makeAction((void*)0x1111, "bar", Nil{}, 42);

  QValues q;
  q[a1] = 5.55;

  EXPECT_EQ(q.find(a2), q.end());
}
