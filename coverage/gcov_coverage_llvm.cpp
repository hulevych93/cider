// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "gcov_coverage_llvm.h"

#include <nlohmann/json.hpp>
#include <process.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace tpl = TinyProcessLib;

namespace cider {
namespace llvm_gc_coverage {

namespace {

class TempGcnoLinks {
 public:
  TempGcnoLinks(const std::filesystem::path& gcnoCppPath) {
    try {
      namespace fs = std::filesystem;

      baseDir = gcnoCppPath.parent_path();

      std::string stem = gcnoCppPath.stem().stem().string();

      fs::path gcdaCppPath = gcnoCppPath;
      gcdaCppPath.replace_extension(".gcda");

      targets = {{gcnoCppPath, baseDir / (stem + ".gcno")},
                 {gcdaCppPath, baseDir / (stem + ".gcda")}};

      for (auto& tarIt : targets) {
        const auto& src = tarIt.first;
        const auto& link = tarIt.second;

        if (!fs::exists(src)) {
          std::cerr << "[TempGcnoLinks] source not found: " << src << "\n";
          continue;
        }

        if (fs::exists(link)) {
          continue;
        }

        try {
          fs::create_symlink(src, link);
          created.push_back(link);
        } catch (const std::exception& e) {
          std::cerr << "[TempGcnoLinks] symlink error: " << e.what() << " for "
                    << link << " -> " << src << "\n";
        }
      }
    } catch (const std::exception& e) {
      std::cerr << "[TempGcnoLinks] init error: " << e.what() << "\n";
    }
  }

  ~TempGcnoLinks() {
    for (auto& link : created) {
      try {
        if (std::filesystem::exists(link)) {
          std::filesystem::remove(link);
        } else {
          std::cerr << "[TempGcnoLinks] link missing at cleanup: " << link
                    << "\n";
        }
      } catch (const std::exception& e) {
        std::cerr << "[TempGcnoLinks] remove error: " << e.what() << " for "
                  << link << "\n";
      }
    }
  }

  TempGcnoLinks(const TempGcnoLinks&) = delete;
  TempGcnoLinks& operator=(const TempGcnoLinks&) = delete;

 private:
  std::filesystem::path baseDir;
  std::vector<std::filesystem::path> created;
  std::vector<std::pair<std::filesystem::path, std::filesystem::path>> targets;
};

std::optional<std::filesystem::path> findGcno(
    const std::filesystem::path& buildRoot,
    const std::string& sourceName) {
  for (auto& p : std::filesystem::recursive_directory_iterator(buildRoot)) {
    if (p.is_regular_file() && p.path().extension() == ".gcno") {
      if (p.path().filename().string().find(sourceName) != std::string::npos) {
        return p.path();
      }
    }
  }
  return std::nullopt;
}

}  // namespace

void GcovBranchParser::feed(const char* data, size_t size) {
  buffer.append(data, size);
  size_t pos = 0;

  while (true) {
    size_t newline = buffer.find('\n', pos);
    if (newline == std::string::npos) {
      buffer.erase(0, pos);
      return;
    }
    std::string line = buffer.substr(pos, newline - pos);
    parseLine(line);
    pos = newline + 1;
  }
}

gcov_coverage::RootReport GcovBranchParser::finish() {
  if (!buffer.empty()) {
    parseLine(buffer);
    buffer.clear();
  }

  gcov_coverage::RootReport root{};
  if (totalBranches > 0) {
    root.report.branchCov.total = totalBranches;
    root.report.branchCov.covered = coveredBranches;
    root.report.branchCov.percent = 100.0 *
                                    static_cast<double>(coveredBranches) /
                                    static_cast<double>(totalBranches);
  }

  return root;
}

void GcovBranchParser::parseLine(const std::string& line) {
  if (line.find("branch") != std::string::npos) {
    totalBranches++;
    if (line.find("never executed") == std::string::npos) {
      size_t taken = 0;
      if (sscanf(line.c_str(), "branch %*d taken %zu", &taken) == 1 &&
          taken > 0) {
        coveredBranches++;
      }
    }
  }
}

bool cleanCoverage(const std::string& workingDir) {
  namespace fs = std::filesystem;
  try {
    for (auto& p : fs::recursive_directory_iterator(workingDir)) {
      const auto& ext = p.path().extension();
      if (ext == ".gcda") {
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
  tpl::Process process(
      binary, workingDir, callback,
      [](const char*, std::size_t) {},  // stderr callback
      true);                            // open_stdin

  process.write(script.data(), script.size());
  process.close_stdin();

  return process.get_exit_status() == 0;
}

namespace {

bool runCoveragePerSource(
    const std::string& objectDir,
    const std::string& sourceFilePath,
    std::function<void(const char*, std::size_t)> callback) {
  auto gcnoOpt =
      findGcno(objectDir, std::filesystem::path(sourceFilePath).filename());
  if (!gcnoOpt) {
    std::cerr << "[runCoverageGcov] .gcno not found for " << sourceFilePath
              << "\n";
    return false;
  }

  const auto& pathToCheck = std::filesystem::path(sourceFilePath).parent_path();

  std::filesystem::path gcnoPath = *gcnoOpt;

  TempGcnoLinks links(gcnoPath);
  (void)links;

  std::string cmd = "xcrun llvm-cov gcov -b -t -r -s " + pathToCheck.string() +
                    " -o " + gcnoPath.parent_path().string() + " " +
                    sourceFilePath;

  tpl::Process process(
      cmd, objectDir, [&](const char* data, size_t n) { callback(data, n); },
      [&](const char* data, size_t n) {
        std::cerr << "[stderr] " << std::string(data, n);
      },
      false);

  auto result = process.get_exit_status() == 0;

  std::filesystem::path gcdaPath = gcnoPath.replace_extension(".gcda");
  std::filesystem::remove(gcdaPath);
  return result;
}

}  // namespace

bool runCoverage(const std::string& objectDir,
                 const std::string& sourcesDir,
                 std::function<void(const char*, std::size_t)> callback) {
  namespace fs = std::filesystem;
  bool ok = true;

  try {
    for (auto& entry : fs::recursive_directory_iterator(sourcesDir)) {
      if (!entry.is_regular_file())
        continue;

      auto path = entry.path();
      if (path.extension() == ".cpp" || path.extension() == ".cc" ||
          path.extension() == ".cxx") {
        if (!runCoveragePerSource(objectDir, path.string(), callback)) {
          std::cerr << "[runCoverageForAllSources] failed for " << path << "\n";
          ok = false;
        }
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "[runCoverageForAllSources] error: " << e.what() << "\n";
    return false;
  }

  return ok;
}

}  // namespace llvm_gc_coverage
}  // namespace cider
