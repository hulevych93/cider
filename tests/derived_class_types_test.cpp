// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "utils/test_suite.h"

#include "derived_class_types.h"

using namespace cider::recorder;
using namespace cider::models;
using namespace cider::tests;
using namespace cider;

struct DerivedClassTypesTest : TestSuite {};

const char* derived_class_types_test_script =
    R"(local object_1 = example.DerivedClass('false')
object_1:sayGoodbye(example.Int(2))
)";

TEST_F(DerivedClassTypesTest, derived_class_types_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::DerivedClass object("false");
  object.sayGoodbye(2);

  SCOPED_TRACE("derived_class_types_test_script");
  testScript(derived_class_types_test_script, session);
}

const char* some_base_class_types_test_script =
    R"(local object_1 = example.BaseClass('true')
object_1:sayHello()
)";

TEST_F(DerivedClassTypesTest, some_base_class_types_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::BaseClass object("true");
  object.sayHello();

  SCOPED_TRACE("some_base_class_types_test_script");
  testScript(some_base_class_types_test_script, session);
}

const char* some_base_class_get_types_test_script =
    R"(local object_1 = example.BaseClass('true')
local object_2 = object_1:get()
object_2:sayGoodbye(example.Int(3))
)";

TEST_F(DerivedClassTypesTest, some_base_class_get_types_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::BaseClass object("true");
  auto derived = object.get();
  derived.sayGoodbye(3);

  SCOPED_TRACE("some_base_class_get_types_test_script");
  testScript(some_base_class_get_types_test_script, session);
}

const char* some_base_class_get_const_types_test_script =
    R"(local object_1 = example.BaseClass('true')
local object_2 = object_1:get()
object_2:sayHello()
)";

TEST_F(DerivedClassTypesTest, some_base_class_get_const_types_test) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  const hook::BaseClass object("true");
  auto base = object.get();
  base.sayHello();

  SCOPED_TRACE("some_base_class_get_const_types_test_script");
  testScript(some_base_class_get_const_types_test_script, session);
}
