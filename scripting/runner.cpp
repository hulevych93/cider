// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "runner.h"

#include <iostream>
#include <process.hpp>

namespace cider {
namespace scripting {

LuaWorker::LuaWorker(const std::string& binary, const std::string& workingDir) {
  process = std::make_unique<TinyProcessLib::Process>(
      binary, workingDir,
      [this](const char* bytes, size_t n) {
        {
          std::lock_guard<std::mutex> lk(mtx);
          buffer.append(bytes, n);
          if (buffer.find("\nEND\n") != std::string::npos) {
            ready = true;
          }
        }
        cv.notify_one();
      },
      [this](const char*, size_t) {
        //std::cerr << "[stderr] " << std::string(bytes, n) << std::endl;
      },
      true);
  _binary = binary;
  _workingDir = workingDir;
}

LuaWorker::~LuaWorker() {
  if (process) {
    write("QUIT\n");
    process->get_exit_status();
  }
}

bool LuaWorker::runScript(
    const std::string& script,
    std::function<void(const char*, std::size_t)> callback) {
  {
    std::unique_lock<std::mutex> lk(mtx);
    (void)(lk);

    buffer.clear();
    ready = false;
  }

  write("RUN\n");
  write(script);
  write("\n<<<END>>>\n");

  std::unique_lock<std::mutex> lk(mtx);
  bool finished =
      cv.wait_for(lk, std::chrono::seconds(3), [&] { return ready.load(); });

  if (!finished) {
    int exit_code = process->get_exit_status();
    if (exit_code != -1) {
      process.reset(new TinyProcessLib::Process(
          _binary, _workingDir,
          [this](const char* bytes, size_t n) {
            {
              std::lock_guard<std::mutex> lk(mtx);
              (void)(lk);

              buffer.append(bytes, n);
              if (buffer.find("\nEND\n") != std::string::npos) {
                ready = true;
              }
            }
            cv.notify_one();
          },
          [this](const char* bytes, size_t n) {
            std::cerr << "[stderr] " << std::string(bytes, n) << std::endl;
          },
          true));
    }

    return false;
  }

  if (callback) {
    callback(buffer.data(), buffer.size());
  }

  bool success = buffer.find("OK") != std::string::npos;
  buffer.clear();
  return success;
}

void LuaWorker::write(const std::string& msg) {
  process->write(msg.c_str(), msg.size());
}

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
