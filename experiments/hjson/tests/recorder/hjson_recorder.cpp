#include "recorder/recorder.h"

#include <fstream>
#include <iostream>

#include "hjson_test.h"

void test_value();
void test_marshal();

int main(int argc, char* argv[]) {
  assert(argc == 2);
  std::cout << "working directory: " << argv[1];

  auto session = cider::recorder::makeLuaRecordingSession(
      "hjson", cider::recorder::SessionSettings{67});

  try {
    test_marshal();
  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  try {
    const auto script = session->getScript();
    session.reset();

    std::ofstream scr1(argv[1]);
    scr1 << script;
  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  return 0;
}
