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
                     const std::string& name,
                     const cider::coverage::CoverageReport& report) {
  ss << name << "\t" << report.lineCov.percent << "\t"
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

std::string generateScript(
    const std::vector<cider::recorder::Action>& actions) {
  auto generator = cider::recorder::makeLuaGenerator("hjson");
  try {
    for (const auto& action : actions) {
      std::visit(generator, action);
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
      test_value();
      test_marshal();
    } catch (const std::exception& e) {
      std::cout << e.what();
    }

    // std::ofstream report("report.txt");

    cider::harmony::Settings settings;
    settings.mutationRate = 0.1;
    settings.harmonyMemoryConsiderationRate = 0.1;
    settings.harmonyMemorySize = 3;
    settings.maxIterations = 10000;
    settings.strategy = cider::harmony::MutationStrategy::ChangeBits;

    settings.meassure = [&](const std::vector<cider::recorder::Action>& actions)
        -> cider::harmony::ReportOpt {
      const auto script = generateScript(actions);

      assert(cleanCoverage());

      const auto result = runScript(cmd.binPath, cmd.workingDir, script);
      if (result) {
        std::string jsonReport;
        assert(runCoverage(cmd.baseDir, cmd.objectDir,
                           [&](const char* data, std::size_t size) {
                             jsonReport += std::string{data, size};
                           }));

        const auto rootReport =
            cider::coverage::parseJsonCovReport(jsonReport, false);
        assert(rootReport.has_value());

        printTableEntry(std::cout, "root", rootReport->report);

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
