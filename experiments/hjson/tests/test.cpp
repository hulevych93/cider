#include "recorder/recorder.h"

#include <fstream>
#include <iostream>

#include "hjson_test.h"

#include "scripting/interpreter.h"

#ifdef __cplusplus
extern "C" {
#endif

int luaopen_hjson(lua_State* L);

#ifdef __cplusplus
}
#endif

void test_value();

int main() {
  auto session = cider::recorder::makeLuaRecordingSession("hjson");

  try {
    test_value();
  } catch (const std::exception& e) {
    std::cerr << e.what();
  }

  const auto script = session->getScript();
  session.reset();

  std::ofstream scr1("script.lua");
  scr1 << script;

  auto lState = cider::scripting::get_lua();
  luaopen_hjson(lState.get());

  assert(cider::scripting::executeScript(lState.get(), script.c_str()));

  return 0;
}
