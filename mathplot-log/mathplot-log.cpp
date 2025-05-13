// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "mathplot-log.h"

#include <assert.h>

#include <iostream>

#include "recorder/details/generator.h"

#ifdef ENABLE_MATHPLOT
#include <matplotlibcpp.h>

namespace plt = matplotlibcpp;
#endif

namespace cider {

namespace {

std::string ensurePngExtension(const std::string& path) {
  std::filesystem::path filePath(path);

  // Check if the extension is already .png (case insensitive)
  if (filePath.extension() == ".png") {
    return filePath.string();
  }

  // Add .png extension
  filePath.replace_extension(".png");
  return filePath.string();
}

std::string ensurePath(const std::string& logDir,
                       const std::string& logFileName) {
  std::filesystem::path outPath(logDir);
  std::filesystem::create_directories(outPath);
  outPath /= logFileName;
  return outPath.string();
}

}  // namespace

namespace cfg_coverage {

MathplotLogger::MathplotLogger(const std::string& logDir,
                               const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

void MathplotLogger::log(size_t index, const Coverage& coverage) const {
  index_.push_back(static_cast<double>(index));
  _percents.push_back(coverage.getPercentage());

  plt::clf();                    // Clear previous frame
  plt::plot(index_, _percents);  // Plot updated points
  plt::xlabel("Iteration");
  plt::ylabel("CFG Coverage (%)");
  plt::title(" ");
  plt::grid(true);
  plt::pause(0.01);  // Allow time for GUI to update
}

MathplotLogger::~MathplotLogger() {
  plt::save(ensurePngExtension(m_path), 1200);
}

}  // namespace cfg_coverage

namespace gcov_coverage {

MathplotLogger::MathplotLogger(const std::string& logDir,
                               const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}
MathplotLogger::~MathplotLogger() {
  plt::save(ensurePngExtension(m_path), 1200);
}

void MathplotLogger::log(size_t index, const RootReport& coverage) const {
  i_.push_back(static_cast<double>(index));
  lcov_.push_back(coverage.report.lineCov.percent);
  bcov_.push_back(coverage.report.branchCov.percent);
  fcov_.push_back(coverage.report.funcCov.percent);

  plt::clf();  // Clear previous frame

  // Plot each coverage vector with labels
  plt::plot(i_, lcov_,
            std::map<std::string, std::string>{{"label", "Line Coverage"}});
  plt::plot(i_, bcov_,
            std::map<std::string, std::string>{{"label", "Branch Coverage"}});
  plt::plot(i_, fcov_,
            std::map<std::string, std::string>{{"label", "Function Coverage"}});

  plt::xlabel("Iteration");
  plt::ylabel("Coverage (%)");
  plt::title(" ");
  plt::grid(true);
  plt::legend();     // Show legend with labels
  plt::pause(0.01);  // Allow time for GUI to update
}

ComparativeLogger::ComparativeLogger(const std::string& logDir,
                                     const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

ComparativeLogger::~ComparativeLogger() {
  plt::save(ensurePngExtension(m_path), 1200);
}

void ComparativeLogger::log(size_t index, const RootReport& coverage) const {
  if (m_first) {
    i_.push_back(static_cast<double>(index));
    ilcov_.push_back(coverage.report.lineCov.percent);
    ibcov_.push_back(coverage.report.branchCov.percent);
  } else {
    j_.push_back(static_cast<double>(index));
    jlcov_.push_back(coverage.report.lineCov.percent);
    jbcov_.push_back(coverage.report.branchCov.percent);
  }

  plot();
}

void ComparativeLogger::plot() const {
  plt::clf();  // Clear previous frame

  // Plot each coverage vector with labels
  plt::plot(i_, ilcov_,
            std::map<std::string, std::string>{{"label", "LCOV"},
                                               {"color", "red"},
                                               {"linestyle", "-"},
                                               {"linewidth", "1.0"}});
  plt::plot(i_, ibcov_,
            std::map<std::string, std::string>{{"label", "BRCOV"},
                                               {"color", "green"},
                                               {"linestyle", "-"},
                                               {"linewidth", "1.0"}});

  plt::plot(j_, jlcov_,
            std::map<std::string, std::string>{{"label", "LCOV Optimized"},
                                               {"color", "red"},
                                               {"linestyle", "--"},
                                               {"linewidth", "1.0"}});
  plt::plot(j_, jbcov_,
            std::map<std::string, std::string>{{"label", "BRCOV Optimized"},
                                               {"color", "green"},
                                               {"linestyle", "--"},
                                               {"linewidth", "1.0"}});

  plt::xlabel("Script instruction number");
  plt::ylabel("Coverage (%)");
  plt::title(" ");
  plt::grid(true);
  plt::legend();     // Show legend with labels
  plt::pause(0.01);  // Allow time for GUI to update
}

TripleComparativeLogger::TripleComparativeLogger(const std::string& logDir,
                                                 const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

TripleComparativeLogger::~TripleComparativeLogger() {
  plt::save(ensurePngExtension(m_path), 1200);
}

void TripleComparativeLogger::log(size_t index,
                                  const RootReport& coverage) const {
  if (_md == 0) {
    i_.push_back(static_cast<double>(index));
    ilcov_.push_back(coverage.report.lineCov.percent);
    ibcov_.push_back(coverage.report.branchCov.percent);
  } else if (_md == 1) {
    j_.push_back(static_cast<double>(index));
    jlcov_.push_back(coverage.report.lineCov.percent);
    jbcov_.push_back(coverage.report.branchCov.percent);
  } else {
    k_.push_back(static_cast<double>(index));
    klcov_.push_back(coverage.report.lineCov.percent);
    kbcov_.push_back(coverage.report.branchCov.percent);
  }

  plot();
}

void TripleComparativeLogger::plot() const {
  plt::clf();  // Clear previous frame

  // Plot each coverage vector with labels
  plt::plot(i_, ilcov_,
            std::map<std::string, std::string>{{"label", "LCOV"},
                                               {"color", "red"},
                                               {"linestyle", "-"},
                                               {"marker", "o"},
                                               {"markersize", "2.0"},
                                               {"linewidth", "1.0"}});
  plt::plot(i_, ibcov_,
            std::map<std::string, std::string>{{"label", "BRCOV"},
                                               {"color", "green"},
                                               {"linestyle", "-"},
                                               {"marker", "o"},
                                               {"markersize", "2.0"},
                                               {"linewidth", "1.0"}});

  plt::plot(j_, jlcov_,
            std::map<std::string, std::string>{{"label", "LCOV Target"},
                                               {"color", "red"},
                                               {"linestyle", "--"},
                                               {"marker", "x"},
                                               {"markersize", "2.0"},
                                               {"linewidth", "1.0"}});
  plt::plot(j_, jbcov_,
            std::map<std::string, std::string>{{"label", "BRCOV Target"},
                                               {"color", "green"},
                                               {"linestyle", "--"},
                                               {"marker", "x"},
                                               {"markersize", "2.0"},
                                               {"linewidth", "1.0"}});

  plt::plot(k_, klcov_,
            std::map<std::string, std::string>{{"label", "LCOV RAND"},
                                               {"color", "red"},
                                               {"linestyle", "-."},
                                               {"marker", "v"},
                                               {"markersize", "2.0"},
                                               {"linewidth", "1.0"}});
  plt::plot(k_, kbcov_,
            std::map<std::string, std::string>{{"label", "BRCOV RAND"},
                                               {"color", "green"},
                                               {"linestyle", "-."},
                                               {"marker", "v"},
                                               {"markersize", "2.0"},
                                               {"linewidth", "1.0"}});

  plt::xlabel("Script instruction number");
  plt::ylabel("Coverage (%)");
  plt::title(" ");
  plt::grid(true);
  plt::legend();     // Show legend with labels
  plt::pause(0.01);  // Allow time for GUI to update
}

}  // namespace gcov_coverage

namespace metasearch {

MathplotLogger::MathplotLogger(const std::string& logDir,
                               const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

void MathplotLogger::log(size_t index, const Solution& solution) const {
  x_.push_back(static_cast<double>(index));
  y_.push_back(solution.objVal);

  plt::clf();         // Clear previous frame
  plt::plot(x_, y_);  // Plot updated points
  plt::xlabel("Iteration");
  plt::ylabel("CFG Coverage (%)");
  plt::title(" ");
  plt::grid(true);
  plt::pause(0.01);  // Allow time for GUI to update
}

MathplotLogger::~MathplotLogger() {
  plt::save(ensurePngExtension(m_path), 1200);
}

}  // namespace metasearch

namespace qleaning {

MathplotLogger::MathplotLogger(const std::string& logDir,
                               const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {
  plt::figure_size(640, 640);
}

void MathplotLogger::logReward(size_t episode, const double totalReward) const {
  ieps_.push_back(static_cast<double>(episode));
  rwrd_.push_back(totalReward);

  plot();
}

void MathplotLogger::logLoss(size_t episode, const double averageLoss) const {
  jeps_.push_back(static_cast<double>(episode));
  loss_.push_back(averageLoss);

  plot();
}

void MathplotLogger::logLR(size_t episode, double learningRate) const {
  keps_.push_back(static_cast<double>(episode));
  lr_.push_back(learningRate);

  plot();
}

void MathplotLogger::plot() const {
  plt::clf();

  plt::subplot2grid(3, 1, 0, 0);
  plt::plot(ieps_, rwrd_,
            std::map<std::string, std::string>{{"label", "Total Reward"},
                                               {"color", "red"},
                                               {"linestyle", "-"},
                                               {"linewidth", "0.5"}});
  plt::grid(true);
  plt::legend();

  plt::subplot2grid(3, 1, 1, 0);
  plt::plot(jeps_, loss_,
            std::map<std::string, std::string>{{"label", "Average Loss"},
                                               {"color", "blue"},
                                               {"linestyle", "-"},
                                               {"linewidth", "0.5"}});
  plt::grid(true);
  plt::legend();

  plt::subplot2grid(3, 1, 2, 0);
  plt::plot(keps_, lr_,
            std::map<std::string, std::string>{{"label", "Learning Rate"},
                                               {"color", "green"},
                                               {"linestyle", "-"},
                                               {"linewidth", "0.5"}});
  plt::xlabel("Epochs");

  plt::legend();
  plt::grid(true);

  plt::pause(0.01);
}

MathplotLogger::~MathplotLogger() {
  plt::save(ensurePngExtension(m_path), 1200);
}

}  // namespace qleaning

}  // namespace cider
