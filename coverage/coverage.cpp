// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "coverage.h"

#include <assert.h>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>

namespace cider {

Cmd::Cmd(int argc, char* argv[]) {
  assert(argc >= 9);

  pipelineType = static_cast<PipelineType>(std::atoi(argv[1]));
  workingDir = argv[2];
  sourcesDir = argv[3];
  objectDir = argv[4];
  binPath = argv[5];
  covDir = argv[6];
  resultsDir = argv[7];
  commonResultsDir = argv[8];

  if (argc > 9) {
    group = static_cast<MethodsGroup>(std::atoi(argv[9]));
  }

  std::cout << (int)pipelineType << " " << workingDir << " " << sourcesDir
            << " " << objectDir << " " << binPath << " " << covDir << " "
            << resultsDir << " " << commonResultsDir << " " << (int)group
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

bool isDebuggerAttached() {
    int mib[4];
    struct kinfo_proc info;
    size_t size = sizeof(info);
    memset(&info, 0, sizeof(info));

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    if (sysctl(mib, 4, &info, &size, nullptr, 0) == -1) {
        return false;  // safer default
    }
    return (info.kp_proc.p_flag & P_TRACED) != 0;
}

}  // namespace cider
