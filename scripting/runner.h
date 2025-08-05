// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <memory>
#include <string>

namespace cider {
namespace scripting {

bool runScript(const std::string& binary,
               const std::string& workingDir,
               const std::string& script,
               std::function<void(const char*, std::size_t)> callback);

}  // namespace scripting
}  // namespace cider
