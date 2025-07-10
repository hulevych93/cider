// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "coverage.h"

#include <assert.h>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace cider {

Cmd::Cmd(int argc, char* argv[]) {
  assert(argc >= 9);

  pipelineType = static_cast<PipelineType>(std::atoi(argv[1]));
  workingDir = argv[2];
  baseDir = argv[3];
  objectDir = argv[4];
  binPath = argv[5];
  covDir = argv[6];
  resultsDir = argv[7];
  commonResultsDir = argv[8];

  if (argc > 9) {
    group = static_cast<MethodsGroup>(std::atoi(argv[9]));
  }

  std::cout << "pipelineType: " << (int)pipelineType
            << ", workingDir: " << workingDir << ", baseDir: " << baseDir
            << ", objectDir: " << objectDir << ", binPath: " << binPath
            << ", covDir: " << covDir << ", resultsDir: " << resultsDir
            << ", group: " << (int)group << std::endl;
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
