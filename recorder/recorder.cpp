#include "recorder.h"

#include "details/session.h"

#include "details/lua/lua_func.h"
#include "details/lua/lua_params.h"

namespace cider {
namespace recorder {

ScriptRecordSessionPtr makeLuaRecordingSession(const std::string& moduleName) {
  LanguageContext context;
  context.funcProducer = lua::produceFunctionCall;
  context.paramProducer = lua::produceParamCode;
  context.binaryOpProducer = lua::produceBinaryOpCall;
  context.functionNameMutator = lua::mutateFunctionName;
  context.unaryOpProducer = lua::produceUnaryOpCall;
  ScriptGenerator generator{moduleName, context};

  auto session =
      std::make_shared<ScriptRecordSessionImpl>(std::move(generator));
  ActionsObservable::getInstance().attachObserver(session);
  return session;
}

}  // namespace recorder
}  // namespace cider
