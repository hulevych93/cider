#include <gtest/gtest.h>

#include "recorder/details/generator.h"
#include "recorder/details/session.h"

#include "recorder/details/lua/lua_func.h"
#include "recorder/details/lua/lua_params.h"

#include <clocale>

using namespace cider::recorder;
using namespace cider;

class GeneratorTestSuite : public testing::Test {
 public:
  class SomeParam {
  } someParam;
  struct SomeOtherNonAgrParam {
    SomeOtherNonAgrParam() {}
  } someOtherParam;

  const Action function = makeAction("some_func", 28, someParam);
  const Action someOtherFunc = makeAction("some_func_other", someOtherParam);
  const Action memberFunction =
      makeAction((void*)nullptr, "some_func", 28, true);
  const Action binaryOp =
      makeAction((void*)nullptr, BinaryOpType::Assignment, someParam);
};

std::string produceAggregateCode(const std::string&,
                                 const GeneratorTestSuite::SomeParam&,
                                 CodeSink& sink) {
  std::string code;
  code += "local {var} = {bar} {car}\n";
  return sink.processLocalVar(std::move(code));
}

TEST_F(GeneratorTestSuite, scriptErrorMsg) {
  LanguageContext context;

  ScriptGenerator generator{"module", context};
  try {
    std::visit(generator, function);
  } catch (const ScriptGenerationError& err) {
    EXPECT_NE("", err.what());
  }
}

TEST_F(GeneratorTestSuite, nullLangContext) {
  LanguageContext context;

  ScriptGenerator generator{"module", context};
  EXPECT_THROW(std::visit(generator, function), ScriptGenerationError);
  EXPECT_THROW(std::visit(generator, memberFunction), ScriptGenerationError);
  EXPECT_THROW(std::visit(generator, binaryOp), ScriptGenerationError);
}

TEST_F(GeneratorTestSuite, nullParamProcessor) {
  LanguageContext context;
  context.funcProducer = lua::produceFunctionCall;
  context.binaryOpProducer = lua::produceBinaryOpCall;

  ScriptGenerator generator{"module", context};
  EXPECT_THROW(std::visit(generator, function), ScriptGenerationError);
}

TEST_F(GeneratorTestSuite, nullContextForSession) {
  LanguageContext context;
  ScriptGenerator generator{"module", context};

  ScriptRecordSessionImpl session{std::move(generator)};
  static_cast<ActionsObserver&>(session).onActionBegins(function);

  EXPECT_THROW(session.getScript(), ScriptGenerationError);
}

TEST_F(GeneratorTestSuite, invalidTemplateGenerator) {
  LanguageContext context;
  context.funcProducer = lua::produceFunctionCall;
  context.binaryOpProducer = lua::produceBinaryOpCall;
  context.paramProducer = lua::produceParamCode;

  ScriptGenerator generator{"module", context};
  EXPECT_THROW(std::visit(generator, function), ScriptGenerationError);
  EXPECT_THROW(std::visit(generator, memberFunction), ScriptGenerationError);
  EXPECT_THROW(std::visit(generator, binaryOp), ScriptGenerationError);
}

TEST_F(GeneratorTestSuite, objectCantBeRegisteredTwice) {
  LanguageContext context;
  context.funcProducer = lua::produceFunctionCall;
  context.binaryOpProducer = lua::produceBinaryOpCall;
  context.paramProducer = lua::produceParamCode;

  ScriptGenerator generator{"module", context};
  EXPECT_NO_THROW(std::visit(generator, someOtherFunc));
  EXPECT_THROW(std::visit(generator, someOtherFunc), ScriptGenerationError);
}
