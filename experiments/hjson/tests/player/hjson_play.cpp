#include <fstream>
#include <iostream>

#include "scripting/interpreter.h"

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

int luaopen_hjson(lua_State* L);

#ifdef __cplusplus
}
#endif

int main(int argc, char* argv[]) {
  try {
    assert(argc == 2);
    std::cout << "running script: " << argv[1];

    std::ifstream scr1(argv[1], std::ios::binary);
    scr1.seekg(0, std::ios::end);
    size_t size = scr1.tellg();
    std::string script(size, ' ');
    scr1.seekg(0);
    scr1.read(&script[0], size);

    auto lState = cider::scripting::get_lua();
    luaopen_hjson(lState.get());

    cider::scripting::executeScript(lState.get(), script.c_str());

  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  return 0;
}
