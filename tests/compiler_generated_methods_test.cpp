
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

// const char* no_constructors_move_assgn_test_script =
//     R"(local object_1 = example.NoConstructors()
// local object_2 = example.NoConstructors()
// object_2 = object_1
// object_2:doo()
//)";

// TEST_F(CompilerGeneratedMethods, moveAssignNoConstructors) {
//   auto session = makeLuaRecordingSession(LuaExampleModuleName);

//  hook::NoConstructors object;
//  hook::NoConstructors object2;
//  object2 = std::move(object);
//  object2.doo();

//  SCOPED_TRACE("no_constructors_move_assgn_test_script");
//  testScript(no_constructors_move_assgn_test_script, session);
//}

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

const char* default_and_move_ctr_test_script =
    R"(local object_1 = example.DefaultAndMoveConstructor()
object_1:doo()
)";

TEST_F(CompilerGeneratedMethods, defaultConstructorDefaultAndMoveConstructor) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::DefaultAndMoveConstructor object;
  object.doo();

  SCOPED_TRACE("default_and_move_ctr_test_script");
  testScript(default_and_move_ctr_test_script, session);
}

const char* move_default_and_move_ctr_test_script =
    R"(local object_1 = example.DefaultAndMoveConstructor()
object_1:doo()
)";

TEST_F(CompilerGeneratedMethods, moveConstructorDefaultAndMoveConstructor) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::DefaultAndMoveConstructor object;
  hook::DefaultAndMoveConstructor object2(std::move(object));
  object2.doo();

  SCOPED_TRACE("move_default_and_move_ctr_test_script");
  testScript(move_default_and_move_ctr_test_script, session);
}

const char* default_and_copy_ctr_test_script =
    R"(local object_1 = example.DefaultAndCopyConstructor()
object_1:doo()
)";

TEST_F(CompilerGeneratedMethods, defaultConstructorDefaultAndCopyConstructor) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::DefaultAndCopyConstructor object;
  object.doo();

  SCOPED_TRACE("default_and_copy_ctr_test_script");
  testScript(default_and_copy_ctr_test_script, session);
}

const char* copy_default_and_copy_ctr_test_script =
    R"(local object_1 = example.DefaultAndCopyConstructor()
local object_2 = example.DefaultAndCopyConstructor(object_1)
object_2:doo()
)";

TEST_F(CompilerGeneratedMethods, copyConstructorDefaultAndCopyConstructor) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::DefaultAndCopyConstructor object;
  hook::DefaultAndCopyConstructor object2(object);
  object2.doo();

  SCOPED_TRACE("copy_default_and_copy_ctr_test_script");
  testScript(copy_default_and_copy_ctr_test_script, session);
}

const char* copy_assgn_default_and_copy_ctr_test_script =
    R"(local object_1 = example.DefaultAndCopyConstructor()
local object_2 = example.DefaultAndCopyConstructor()
object_2 = object_1
object_2:doo()
)";

TEST_F(CompilerGeneratedMethods, copyAssgnDefaultAndCopyConstructor) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::DefaultAndCopyConstructor object;
  hook::DefaultAndCopyConstructor object2;
  object2 = object;
  object2.doo();

  SCOPED_TRACE("copy_assgn_default_and_copy_ctr_test_script");
  testScript(copy_assgn_default_and_copy_ctr_test_script, session);
}

const char* only_copy_operator_ctr_test_script =
    R"(local object_1 = example.OnlyCopyOperator()
object_1:doo()
)";

TEST_F(CompilerGeneratedMethods, defaultConstructorOnlyCopyOperator) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::OnlyCopyOperator object;
  object.doo();

  SCOPED_TRACE("only_copy_operator_ctr_test_script");
  testScript(only_copy_operator_ctr_test_script, session);
}

const char* only_copy_operator_copy_ctr_test_script =
    R"(local object_1 = example.OnlyCopyOperator()
local object_2 = example.OnlyCopyOperator(object_1)
object_2:doo()
)";

TEST_F(CompilerGeneratedMethods, copyConstructorOnlyCopyOperator) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::OnlyCopyOperator object;
  hook::OnlyCopyOperator object2(object);
  object2.doo();

  SCOPED_TRACE("only_copy_operator_copy_ctr_test_script");
  testScript(only_copy_operator_copy_ctr_test_script, session);
}

const char* copy_assgn_only_copy_operator_ctr_test_script =
    R"(local object_1 = example.OnlyCopyOperator()
local object_2 = example.OnlyCopyOperator()
object_2 = object_1
object_2:doo()
)";

TEST_F(CompilerGeneratedMethods, copyAssgnOnlyCopyOperator) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::OnlyCopyOperator object;
  hook::OnlyCopyOperator object2;
  object2 = object;
  object2.doo();

  SCOPED_TRACE("copy_assgn_only_copy_operator_ctr_test_script");
  testScript(copy_assgn_only_copy_operator_ctr_test_script, session);
}

const char* only_move_operator_ctr_test_script =
    R"(local object_1 = example.OnlyMoveOperator()
object_1:doo()
)";

TEST_F(CompilerGeneratedMethods, defaultConstructorOnlyMoveOperator) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::OnlyMoveOperator object;
  object.doo();

  SCOPED_TRACE("only_move_operator_ctr_test_script");
  testScript(only_move_operator_ctr_test_script, session);
}

const char* move_assgn_only_move_operator_ctr_test_script =
    R"(local object_1 = example.OnlyMoveOperator()
local object_2 = example.OnlyMoveOperator()
object_2 = object_1
object_2:doo()
)";

TEST_F(CompilerGeneratedMethods, moveAssgnOnlyMoveOperator) {
  auto session = makeLuaRecordingSession(LuaExampleModuleName);

  hook::OnlyMoveOperator object;
  hook::OnlyMoveOperator object2;
  object2 = std::move(object);
  object2.doo();

  SCOPED_TRACE("move_assgn_only_move_operator_ctr_test_script");
  testScript(move_assgn_only_move_operator_ctr_test_script, session);
}
