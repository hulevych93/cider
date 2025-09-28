// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "coverage/coverage.h"
#include "coverage/gcov_coverage.h"

namespace cider {
namespace llvm_coverage {

std::optional<gcov_coverage::RootReport> parseJsonCovReport(
    const std::string& json,
    bool deep = false);

bool cleanCoverage(const std::string& workingDir);

bool runScript(const std::string& binary,
               const std::string& workingDir,
               const std::string& script,
               std::function<void(const char*, std::size_t)> callback);

bool runCoverage(const std::string& base,
                 const std::string& objectDir,
                 std::function<void(const char*, std::size_t)> callback);

}  // namespace llvm_coverage
}  // namespace cider
