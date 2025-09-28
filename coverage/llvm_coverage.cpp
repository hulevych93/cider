// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "llvm_coverage.h"

#include <nlohmann/json.hpp>

#include <iostream>
#include <process.hpp>

#include <filesystem>
#include <fstream>

namespace tpl = TinyProcessLib;

namespace cider {
namespace llvm_coverage {

std::optional<gcov_coverage::RootReport> parseJsonCovReport(
    const std::string& json,
    bool deep) {
  try {
    auto j = nlohmann::json::parse(json);

    if (!j.contains("data") || j["data"].empty() ||
        !j["data"][0].contains("totals"))
      return std::nullopt;

    const auto& totals = j["data"][0]["totals"];

    gcov_coverage::RootReport root;

    auto fill = [](const nlohmann::json& src, gcov_coverage::Coverage& dst) {
      dst.total = src.value("count", 0);
      dst.covered = src.value("covered", 0);
      dst.percent = src.value("percent", 0.0);
    };

    fill(totals["lines"], root.report.lineCov);
    fill(totals["branches"], root.report.branchCov);
    fill(totals["functions"], root.report.funcCov);

    return root;
  } catch (const std::exception& e) {
    std::cerr << "[parseJsonCovTotals] " << e.what() << "\n";
    return std::nullopt;
  }
}

bool cleanCoverage(const std::string& workingDir) {
  namespace fs = std::filesystem;
  try {
    for (auto& p : fs::recursive_directory_iterator(workingDir)) {
      const auto& ext = p.path().extension();
      if ((ext == ".profraw") || (ext == ".profdata")) {
        fs::remove(p.path());
      }
    }
    return true;
  } catch (const std::exception& e) {
    std::cerr << "[cleanCoverage] Error: " << e.what() << "\n";
    return false;
  }
}

bool runScript(const std::string& binary,
               const std::string& workingDir,
               const std::string& script,
               std::function<void(const char*, std::size_t)> callback) {
  std::string command =
      "LLVM_PROFILE_FILE=" + workingDir + "/default.profraw " + binary;

  TinyProcessLib::Process process(
      command, workingDir, callback,
      [](const char*, std::size_t) {},  // stderr callback
      true);                            // open_stdin

  process.write(script.data(), script.size());
  process.close_stdin();

  return process.get_exit_status() == 0;
}

bool runCoverage(const std::string& binaryPath,
                 const std::string& workingDir,
                 std::function<void(const char*, std::size_t)> callback) {
  std::string profraw = workingDir + "/default.profraw";
  std::string profdata = workingDir + "/default.profdata";

  std::string merge_cmd =
      "xcrun llvm-profdata merge -sparse " + profraw + " -o " + profdata;
  tpl::Process merge_process(
      merge_cmd, workingDir,
      [](const char* data, size_t n) { std::cout.write(data, n); },
      [](const char* data, size_t n) { std::cerr.write(data, n); }, false);
  if (merge_process.get_exit_status() != 0) {
    std::cerr << "[runCoverage] llvm-profdata merge failed\n";
    // return false;
  }

  std::string cmd =
      "xcrun llvm-cov export --summary-only --skip-expansions "
      "--skip-functions " +
      binaryPath + " --instr-profile=" + profdata;

  tpl::Process cov_process(
      cmd, workingDir, callback,
      [](const char* data, size_t n) { std::cerr.write(data, n); }, false);
  return cov_process.get_exit_status() == 0;
}

}  // namespace llvm_coverage
}  // namespace cider
