#include "test_suite.h"

#include "scripting/interpreter.h"

#ifdef __cplusplus
extern "C" {
#endif

int luaopen_example(lua_State* L);

#ifdef __cplusplus
}
#endif

namespace gunit {
namespace tests {

void TestSuite::testScript(const char* expectedScript,
                           recorder::ScriptRecordSessionPtr& session) {
  const auto script = session->getScript();
  session.reset();

  auto lState = scripting::get_lua();
  luaopen_example(lState.get());
  EXPECT_EQ(expectedScript, script);
  EXPECT_TRUE(scripting::executeScript(lState.get(), script.c_str()));
}

}  // namespace tests
}  // namespace gunit
