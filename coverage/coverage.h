// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace cider {

struct Cmd final {
  Cmd(int argc, char* argv[]);

  std::string workingDir;
  std::string baseDir;
  std::string objectDir;
  std::string binPath;
  std::string covDir;
};

std::string loadFile(const std::string& path);

}  // namespace cider
