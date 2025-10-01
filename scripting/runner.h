// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

namespace TinyProcessLib {
class Process;
}

namespace cider {
namespace scripting {

class LuaWorker final {
 public:
  LuaWorker(const std::string& binary, const std::string& workingDir);

  ~LuaWorker();

  bool runScript(const std::string& script,
                 std::function<void(const char*, std::size_t)> callback);

 private:
  std::unique_ptr<TinyProcessLib::Process> process;
  std::string buffer;

  std::string _binary;
  std::string _workingDir;

  std::atomic<bool> ready{false};
  std::mutex mtx;
  std::condition_variable cv;

  void write(const std::string& msg);
};

bool runScript(const std::string& binary,
               const std::string& workingDir,
               const std::string& script,
               std::function<void(const char*, std::size_t)> callback);

}  // namespace scripting
}  // namespace cider
