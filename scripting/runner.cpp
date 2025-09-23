// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "runner.h"

#include <iostream>
#include <process.hpp>

namespace cider {
namespace scripting {

bool runScript(const std::string& binary,
               const std::string& workingDir,
               const std::string& script,
               std::function<void(const char*, std::size_t)> callback) {
  TinyProcessLib::Process process(
      binary, workingDir, callback, [](const char*, std::size_t) {}, true);
  process.write(script.data(), script.size());
  process.close_stdin();
  return process.get_exit_status() == 0;
}

}  // namespace scripting
}  // namespace cider
