// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "coverage.h"

#include <assert.h>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace cider {

Cmd::Cmd(int argc, char* argv[]) {
  assert(argc == 7);
  workingDir = argv[1];
  baseDir = argv[2];
  objectDir = argv[3];
  binPath = argv[4];
  covDir = argv[5];
  resultsDir = argv[6];

  std::cout << "workingDir: " << workingDir << ", baseDir: " << baseDir
            << ", objectDir: " << objectDir << ", binPath: " << binPath
            << ", covDir: " << covDir << ", resultsDir: " << resultsDir
            << std::endl;
}

std::string loadFile(const std::string& path) {
  if (!std::filesystem::exists(path)) {
    std::cout << path << "doesn't exist" << std::endl;
  }
  std::ifstream scr1(path, std::ios::binary);
  scr1.seekg(0, std::ios::end);
  size_t size = scr1.tellg();
  std::string script(size, ' ');
  scr1.seekg(0);
  scr1.read(&script[0], size);
  return script;
}

}  // namespace cider
