// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "pipeline.h"

#include "agent-q-learning/q-learning-agent.h"
#include "agent-sarsa-learning/sarsa-learning-agent.h"

#include "paths.h"

#include <chrono>
#include <ctime>

#include <tlog.h>
#include <filesystem>
#include <iomanip>
#include <sstream>

#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>

namespace cider {
namespace pipelines {

bool isDebuggerAttached() {
  int mib[4];
  struct kinfo_proc info;
  size_t size;

  // Initialize the flags so that, if sysctl fails, we get a predictable result.
  info.kp_proc.p_flag = 0;

  mib[0] = CTL_KERN;
  mib[1] = KERN_PROC;
  mib[2] = KERN_PROC_PID;
  mib[3] = getpid();

  size = sizeof(info);

  if (sysctl(mib, 4, &info, &size, nullptr, 0) == -1) {
    return true;  // false
  }

  // P_TRACED is set if the process is being debugged
  return true;  // (info.kp_proc.p_flag & P_TRACED) != 0;
}

void stopPoint() {
  if (!isDebuggerAttached()) {
    tlog_info << "Press Enter to continue...\n";
    std::cin.get();
  }
}

bool Pipeline::load(const std::string& resultsPath) {
  namespace fs = std::filesystem;

  const auto& config = getReportConfig(_cmd.group);

  try {
    _results.clear();
    for (const auto& entry : fs::directory_iterator(resultsPath)) {
      if (entry.is_regular_file()) {
        auto path = entry.path();
        auto fileName = path.filename().string();

        if (fileName == "results.img") {
          _results.clear();
          tlog_info << "Loading: " << path.string() << std::endl;
          serialization::Deserializer deserializer(path.string());
          deserializer >> _results;
          _oldResults = false;
          break;
        } else if (path.extension() == ".bin" &&
                   fileName.rfind("results_", 0) == 0) {
          std::string fileName = path.filename().string();
          std::string methodName = fileName.substr(8, fileName.size() - 8 - 4);

          if (std::find(config.cbegin(), config.cend(), methodName) !=
              config.cend()) {
            tlog_info << "Loading: " << path.string() << std::endl;

            MethodResults methodResults;
            serialization::Deserializer deserializer(path.string());
            deserializer >> methodResults;

            _results[methodName] = std::move(methodResults);
          } else {
            tlog_info << "SKIP Loading: " << methodName << std::endl;
          }
          _oldResults = false;
        }
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Load error: " << e.what() << std::endl;
    return false;
  }
  return true;
}

bool Pipeline::save(const std::string& path) const {
  if (_oldResults) {
    try {
      serialization::Serializer serializer;
      serializer << _results;
      serializer.save(path + "/results.img");
    } catch (const std::exception& e) {
      std::cerr << "Load error: " << e.what() << std::endl;
      return false;
    }
  } else {
    namespace fs = std::filesystem;
    try {
      if (!fs::exists(path)) {
        fs::create_directories(path);
      }

      for (const auto& it : _results) {
        const auto& methodName = it.first;
        const auto& methodResults = it.second;

        std::string fileName = path + "/results_" + methodName + ".bin";
        serialization::Serializer serializer;
        serializer << methodResults;
        serializer.save(fileName);
      }
    } catch (const std::exception& e) {
      std::cerr << "Save error: " << e.what() << std::endl;
      return false;
    }
  }
  return true;
}

Pipeline::Pipeline(const std::string& libName, const cider::Cmd& cmd)
    : _libName(libName), _cmd(cmd) {
  agent_model::qlearning::QLearningAgent::setPath(
      paths::getQLearningAgentPath(cmd.resultsDir));

  agent_model::sarsa::SarsaLearningAgent::setPath(
      paths::getSarsaAgentPath(cmd.resultsDir));

  tlog_info << "Load results: " << cmd.resultsDir
            << ", status: " << load(cmd.resultsDir) << std::endl;
}

bool Pipeline::run(SessionsGetter getTS, SessionsGetter getTCs) {
  const auto dateTime = getDatetimeForDirName();
  const auto config = pipelineConfig();

  auto needTS = false;
  for (const auto& pipe : _pipes) {
    needTS |= pipe->needTS();
  }

  std::vector<cider::recorder::ScriptRecordSessionPtr> sessions;
  if (needTS) {
    sessions = getTS();
  } else {
    sessions = getTCs();
  }

  auto scrNum = 0;
  for (const auto& session : sessions) {
    tlog_info << "num: " << scrNum++ << "\t name: " << session->getName()
              << "\t count op: " << session->getInstructionsCount()
              << std::endl;
  }

  stopPoint();

  scrNum = 0;
  for (const auto& session : sessions) {
    tlog_info << "num: " << scrNum << "\t name: " << session->getName()
              << "\t count op: " << session->getInstructionsCount()
              << std::endl;
    const auto metadata = dateTime + '_' + config + '/' + session->getName();

    _input.actions = session->getInstructions();
    _input.testOrLibName = session->getName();

    if (!run(metadata)) {
      tlog_info << "pipeline failed." << std::endl;
    }

    ++scrNum;
  }

  stopPoint();

  clearTrash();

  return true;
}

bool Pipeline::save() {
  const auto& resultsDir = _cmd.resultsDir;
  tlog_info << "Save results: " << resultsDir
            << ", status: " << save(resultsDir) << std::endl;

  // printResultsSummary(_results);

  return true;
}

bool Pipeline::run(const std::string& metadata) {
  for (auto& pipe : _pipes) {
    try {
      if (!pipe->process(metadata, _libName, _cmd)) {
        return false;
      }
    } catch (...) {
    }
  }
  return true;
}

void Pipeline::clearTrash() {
  for (auto& storageIt : _results) {
    auto& results = storageIt.second;
    results.entries.erase(
        std::remove_if(results.entries.begin(), results.entries.end(),
                       [](const Result& r) {
                         return r.oldReport.branchCov.percent <= 0.0 ||
                                r.oldCfgReport.getPercentage() <= 0.0;
                       }),
        results.entries.end());
  }
}

const std::string Pipeline::pipelineConfig() const {
  std::string config;
  for (const auto& pipe : _pipes) {
    config += pipe->getLetter() + '_';
  }
  config.pop_back();
  return config;
}

std::string Pipeline::getDatetimeForDirName() {
  auto now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm tm = *std::localtime(&t);

  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%d");
  return oss.str();
}

}  // namespace pipelines
}  // namespace cider
