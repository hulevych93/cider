// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "coverage/coverage.h"
#include "coverage/gcov_coverage.h"

namespace cider {
namespace llvm_gc_coverage {

bool cleanCoverage(const std::string& workingDir);

class GcovBranchParser final {
 public:
  void feed(const char* data, size_t size);

  gcov_coverage::RootReport finish();

 private:
  void parseLine(const std::string& line);

  std::string buffer;
  size_t totalBranches = 0;
  size_t coveredBranches = 0;
};

bool runScript(const std::string& binary,
               const std::string& workingDir,
               const std::string& script,
               std::function<void(const char*, std::size_t)> callback);

bool runCoverage(const std::string& objectDir,
                 const std::string& sourcesDir,
                 std::function<void(const char*, std::size_t)> callback);

}  // namespace llvm_gc_coverage
}  // namespace cider
