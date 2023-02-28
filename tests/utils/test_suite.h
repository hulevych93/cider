#pragma once

#include <gtest/gtest.h>

#include "recorder/recorder.h"

namespace gunit {
namespace tests {

constexpr const char* LuaExampleModuleName = "example";

class TestSuite : public testing::Test {
 public:
  static void testScript(const char* expectedScript,
                         recorder::ScriptRecordSessionPtr& session);
};

}  // namespace tests
}  // namespace gunit
