#include "test_suite.h"

#include "recorder/details/lua/lua_params.h"
#include "scripting/interpreter.h"

#include "gunit-models/aggregate_struct.h"
#include "gunit-models/derived_agregate_types.h"
#include "gunit-models/enum.h"

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

  auto lState = scripting::get_lua();
  luaopen_example(lState.get());
  EXPECT_EQ(expectedScript, script);
  EXPECT_TRUE(scripting::executeScript(lState.get(), script.c_str()));
}

}  // namespace tests
}  // namespace gunit

namespace gunit {
namespace recorder {

template <>
std::string produceAggregateCode(const models::Aggregate& aggregate,
                                 CodeSink& sink) {
  lua::ParamVisitor visitor;
  std::string code;
  code += "local {var} = example.Aggregate()\n";
  code += "{var}.condition = " + visitor(aggregate.condition) + "\n";
  code += "{var}.number = " + visitor(aggregate.number) + "\n";
  return sink.processLocalVar(std::move(code));
}

template <>
std::string produceAggregateCode(const models::AggregateDerived& aggregate,
                                 CodeSink& sink) {
  lua::ParamVisitor visitor;
  std::string code;
  code += "local {var} = example.AggregateDerived()\n";
  code += "{var}.condition = " + visitor(aggregate.condition) + "\n";
  code += "{var}.number = " + visitor(aggregate.number) + "\n";
  code += "{var}.floatingNumber = " + visitor(aggregate.floatingNumber) + "\n";
  return sink.processLocalVar(std::move(code));
}

template <>
std::string produceAggregateCode(const models::SomeEnumeration& that,
                                 CodeSink&) {
  switch (that) {
    case models::SomeEnumeration::first_value:
      return "example.SomeEnumeration_first_value";
      break;
    case models::SomeEnumeration::second_value:
      return "example.SomeEnumeration_second_value";
      break;
  }
  return {};
}

}  // namespace recorder
}  // namespace gunit
