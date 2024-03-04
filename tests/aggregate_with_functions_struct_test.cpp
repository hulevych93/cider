// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "utils/test_suite.h"

#include "aggregate_with_functions.h"

using namespace cider::recorder;
using namespace cider::models;
using namespace cider::tests;
using namespace cider;

struct AggregateWithFunctionsStructStructTest : TestSuite {};

static const char* test_aggregate_with_functions_test_script =
    R"(object_1 = example.AggregateWithFunctionsStruct()
object_1.field = example.Int(13)
example.test_aggregate_with_functions(object_1)
)";

TEST_F(AggregateWithFunctionsStructStructTest,
       function_test_aggregate_with_functions_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::AggregateWithFunctionsStruct aggregateStruct{13};
  EXPECT_EQ(13, hook::test_aggregate_with_functions(aggregateStruct));

  SCOPED_TRACE("test_aggregate_with_functions_test_script");
  testScript(test_aggregate_with_functions_test_script, session);
}
