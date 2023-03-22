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

TEST_F(CompilerGeneratedMethods, defaultConstructorNoConstructors) {
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

TEST_F(CompilerGeneratedMethods, moveConstructorNoConstructors) {
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

TEST_F(CompilerGeneratedMethods, copyConstructorNoConstructors) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::NoConstructors object;
  hook::NoConstructors object2(object);
  object2.doo();

  SCOPED_TRACE("no_constructors_copy_ctr_test_script");
  testScript(no_constructors_copy_ctr_test_script, session);
}

const char* only_destrutor_default_ctr_test_script =
    R"(local object_1 = example.OnlyDestructor()
object_1:doo()
)";

TEST_F(CompilerGeneratedMethods, defaultConstructorOnlyDestructor) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::OnlyDestructor object;
  object.doo();

  SCOPED_TRACE("only_destrutor_default_ctr_test_script");
  testScript(only_destrutor_default_ctr_test_script, session);
}

const char* only_destructor_move_ctr_test_script =
    R"(local object_1 = example.OnlyDestructor()
local object_2 = example.OnlyDestructor(object_1)
object_2:doo()
)";

TEST_F(CompilerGeneratedMethods, moveConstructorOnlyDestructor) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::OnlyDestructor object;
  hook::OnlyDestructor object2(std::move(object));
  object2.doo();

  SCOPED_TRACE("only_destructor_move_ctr_test_script");
  testScript(only_destructor_move_ctr_test_script, session);
}

const char* only_destructor_copy_ctr_test_script =
    R"(local object_1 = example.OnlyDestructor()
local object_2 = example.OnlyDestructor(object_1)
object_2:doo()
)";

TEST_F(CompilerGeneratedMethods, copyConstructorOnlyDestructor) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::OnlyDestructor object;
  hook::OnlyDestructor object2(object);
  object2.doo();

  SCOPED_TRACE("only_destructor_copy_ctr_test_script");
  testScript(only_destructor_copy_ctr_test_script, session);
}
