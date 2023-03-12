#include "utils/test_suite.h"

#include "compiler_generated_methods.h"

using namespace gunit::recorder;
using namespace gunit::models;
using namespace gunit::tests;
using namespace gunit;

struct CompilerGeneratedMethods : TestSuite {};

const char* no_constructors_default_ctr_test_script =
    R"(local object_1 = example.NoConstructors()
object_1:doo()
)";

TEST_F(CompilerGeneratedMethods, defaultConstructor) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::NoConstructors object;
  object.doo();

  SCOPED_TRACE("no_constructors_default_ctr_test_script");
  testScript(no_constructors_default_ctr_test_script, session);
}

const char* no_constructors_move_ctr_test_script =
    R"(local object_1 = example.NoConstructors()
local object_2 = example.NoConstructors(object_1)
object_2:doo()
)";

TEST_F(CompilerGeneratedMethods, moveConstructor) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::NoConstructors object;
  hook::NoConstructors object2(std::move(object));
  object2.doo();

  SCOPED_TRACE("no_constructors_move_ctr_test_script");
  testScript(no_constructors_move_ctr_test_script, session);
}

const char* no_constructors_copy_ctr_test_script =
    R"(local object_1 = example.NoConstructors()
local object_2 = example.NoConstructors(object_1)
object_2:doo()
)";

TEST_F(CompilerGeneratedMethods, copyConstructor) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::NoConstructors object;
  hook::NoConstructors object2(object);
  object2.doo();

  SCOPED_TRACE("no_constructors_copy_ctr_test_script");
  testScript(no_constructors_copy_ctr_test_script, session);
}

TEST_F(CompilerGeneratedMethods, OnlyCopyConstructorobj) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  gunit::models::OnlyCopyConstructor obf;

  SCOPED_TRACE("no_constructors_copy_ctr_test_script");
  testScript(no_constructors_copy_ctr_test_script, session);
}
