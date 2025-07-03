// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "coverage/logger.h"
#include "metaheuristics/metasearch.h"

#include "q-learning/logger.h"

namespace cider {

enum class PlotType { BrCov, LineCov, Both };

constexpr const char* LineStyles[] = {
    "--",  // dashed line
    "-",   // solid line
    "-.",  // dash-dot line
    ":",   // dotted line
};

constexpr const char* ColorCodes[] = {
    "k",  // black
    "b",  // blue
    "r",  // red
    "g",  // green
    "c",  // cyan
    "m",  // magenta
    "y",  // yellow
};

struct LinesBarPlotData final {
  std::string label;
  size_t oldLines = 0;
  std::vector<size_t> newLinesG2;
  std::vector<size_t> newLinesB2;
};

class LinesBarPlot final {
 public:
  LinesBarPlot(const std::string& logDir, const std::string& logFileName);
  ~LinesBarPlot();

  void init(const std::string& label, size_t oldLines);
  void log(const std::string& label,
           const std::string& method,
           size_t newLines);

  void plot() const;
  void save();

 private:
  std::unordered_map<std::string, LinesBarPlotData> _barData;

  std::string m_path;
  bool m_saved = false;
};

namespace cfg_coverage {

class MathplotLogger : public ICoverageLogger {
 public:
  MathplotLogger(const std::string& logDir, const std::string& logFileName);
  ~MathplotLogger() override;

  void log(size_t index, const Coverage& coverage) const override;

  void plot() const;

 private:
  mutable std::vector<double> index_, _percents;
  std::string m_path;
};

}  // namespace cfg_coverage

namespace gcov_coverage {

struct BoxPlotData final {
  std::string label;
  std::vector<double> data;
  std::vector<size_t> lines;
};

class CoverageBoxPlot final : public ICoverageLogger {
 public:
  CoverageBoxPlot(const std::string& logDir,
                  const std::string& logFileName,
                  PlotType type = PlotType::BrCov);
  ~CoverageBoxPlot() override;

  void log(size_t index, const RootReport& coverage) const override;

  void next(const std::string& label);
  void linesCount(size_t lines);

  void plot() const;
  void save();

 private:
  PlotType _type;
  std::vector<BoxPlotData> _boxData;
  BoxPlotData* _current = nullptr;

  std::string m_path;
  bool m_saved = false;
};

struct BarPlotData final {
  std::string label;
  RootReport report;
  size_t lines = 0;
};

class CoverageBarPlot final : public ICoverageLogger {
 public:
  CoverageBarPlot(const std::string& logDir, const std::string& logFileName);
  ~CoverageBarPlot() override;

  void log(size_t index, const RootReport& coverage) const override;

  void next(const std::string& label);

  void plot() const;
  void save();

 private:
  std::vector<BarPlotData> _barData;
  BarPlotData* _current = nullptr;

  std::string m_path;
  bool m_saved = false;
};

class MathplotLogger : public ICoverageLogger {
 public:
  MathplotLogger(const std::string& logDir, const std::string& logFileName);
  ~MathplotLogger() override;

  void log(size_t index, const RootReport& coverage) const override;

  void plot() const;

 private:
  mutable std::vector<double> i_, lcov_, bcov_, fcov_;
  std::string m_path;
};

struct Points {
  std::vector<double> instructions;

  std::vector<double> lineCov;
  std::vector<double> brCov;
};

class StepperComparativeLogger : public ICoverageLogger {
 public:
  StepperComparativeLogger(const std::string& logDir,
                           const std::string& logFileName,
                           PlotType type = PlotType::BrCov);
  ~StepperComparativeLogger() override;

  void log(size_t index, const RootReport& coverage) const override;

  void next(const std::string& name);

  void plot() const;

 private:
  PlotType _type;
  mutable std::unordered_map<std::string, std::vector<Points>> _graphs;
  std::vector<std::string> _order;
  Points* _current = nullptr;
  int _style = 0;
  int _color = 0;

  std::string m_path;
  mutable size_t _updateCounter = 0;
};

}  // namespace gcov_coverage

namespace metasearch {

class MathplotLogger : public IResultsLogger {
 public:
  MathplotLogger(const std::string& logDir, const std::string& logFileName);
  ~MathplotLogger() override;

  void log(size_t index, const Solution& best) const override;

  void plot() const;

 private:
  mutable std::vector<double> x_, y_;
  std::string m_path;
};

}  // namespace metasearch

namespace qleaning {

class MathplotLogger : public IResultsLogger {
 public:
  MathplotLogger(const std::string& logDir, const std::string& logFileName);
  ~MathplotLogger() override;

  void logReward(size_t episode, const double totalReward) const override;
  void logLoss(size_t episode, const double averageLoss) const override;

  void plot() const;

 private:
  mutable std::vector<double> ieps_, rwrd_;
  mutable std::vector<double> jeps_, loss_;

  std::string m_path;
  mutable size_t _updateCounter = 0;
};

}  // namespace qleaning

}  // namespace cider
