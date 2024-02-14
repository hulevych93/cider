// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include <fstream>
#include <iostream>

#include "coverage/coverage.h"
#include "harmony/harmony.h"
#include "recorder/details/generator.h"
#include "recorder/recorder.h"

#include <process.hpp>

#include <assert.h>

namespace tpl = TinyProcessLib;

bool cleanCoverage() {
  tpl::Process process(
      "find . -name \"*.gcda\" -delete", "",
      [](const char* data, std::size_t) { std::cout << data; },
      [](const char* data, std::size_t) { std::cerr << data; });
  return process.get_exit_status() == 0;
}

bool runScript(const std::string& binDir,
               const std::string& workingDir,
               const std::string& script) {
  tpl::Process process(
      binDir + "/hjson_player", workingDir,
      [](const char* data, std::size_t) {},
      [](const char* data, std::size_t) {}, true);
  process.write(script.data(), script.size());
  process.close_stdin();
  return process.get_exit_status() == 0;
}

bool runCoverage(const std::string& base,
                 const std::string& objectDir,
                 std::function<void(const char*, std::size_t)> callback) {
  tpl::Process process("gcovr --json-summary --json-summary-pretty -r" + base +
                           " --object-directory=" + objectDir,
                       "", callback, nullptr);
  return process.get_exit_status() == 0;
}

void printTableEntry(std::ostream& ss,
                     const size_t index,
                     const cider::coverage::CoverageReport& report) {
  ss << index << "\t" << report.lineCov.percent << "\t"
     << report.branchCov.percent << "\t" << report.funcCov.percent << std::endl;
}

struct Cmd final {
  Cmd(int argc, char* argv[]) {
    assert(argc == 5);
    workingDir = argv[1];
    baseDir = argv[2];
    objectDir = argv[3];
    binPath = argv[4];
  }

  std::string workingDir;
  std::string baseDir;
  std::string objectDir;
  std::string binPath;
};

extern void test_value();
extern void test_marshal();

std::string generateScript(const std::vector<cider::recorder::Action>& actions,
                           const size_t limit) {
  auto generator = cider::recorder::makeLuaGenerator("hjson");
  try {
    size_t count = 0;
    for (const auto& action : actions) {
      std::visit(generator, action);
      if (count >= limit) {
        break;
      }
      ++count;
    }
    return generator.getScript();
  } catch (...) {
    generator.discard();
    throw;
  }
}

int main(int argc, char* argv[]) {
  try {
    Cmd cmd(argc, argv);

    auto session = cider::recorder::makeLuaRecordingSession("hjson");

    try {
      test_marshal();
    } catch (const std::exception& e) {
      std::cout << e.what();
    }

    std::ofstream report("report.txt");

    cider::harmony::Settings settings;
    settings.mutationRate = 0.0;
    settings.harmonyMemoryConsiderationRate = 0;
    settings.harmonyMemorySize = 1;
    settings.maxIterations = 10000;
    settings.strategy = cider::harmony::MutationStrategy::ChangeBits;

    size_t index = 1;
    settings.meassure = [&](const std::vector<cider::recorder::Action>& actions)
        -> cider::harmony::ReportOpt {
      const auto script = generateScript(actions, index);

      assert(cleanCoverage());

      const auto result = runScript(cmd.binPath, cmd.workingDir, script);
      if (result) {
        std::string jsonReport;
        assert(runCoverage(cmd.baseDir, cmd.objectDir,
                           [&](const char* data, std::size_t size) {
                             jsonReport += std::string{data, size};
                           }));

        const auto rootReport =
            cider::coverage::parseJsonCovReport(jsonReport, true);
        assert(rootReport.has_value());

        printTableEntry(report, index, rootReport->report);

        for (const auto& fileReport : rootReport->files) {
          std::ofstream fileStream(fileReport.name, std::ios::app);
          printTableEntry(fileStream, index, fileReport.report);
        }

        index++;
        return rootReport.value();
      }

      return std::nullopt;
    };

    cider::harmony::Search hs{settings};
    hs.initialize(session->getInstructions());
    hs.run();
  } catch (const std::exception& e) {
    std::cerr << e.what();
    return 1;
  }

  return 0;
}
