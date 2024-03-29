// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <gtest/gtest.h>

#include "recorder/recorder.h"

namespace cider {
namespace tests {

constexpr const char* LuaExampleModuleName = "example";

class TestSuite : public testing::Test {
 public:
  static void testScript(const char* expectedScript,
                         recorder::ScriptRecordSessionPtr& session);
};

}  // namespace tests
}  // namespace cider
