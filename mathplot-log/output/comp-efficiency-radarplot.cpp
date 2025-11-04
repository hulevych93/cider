// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "comp-efficiency-radarplot.h"

#include <matplotlibcpp.h>

#include <tlog.h>
#include <iomanip>
#include <sstream>

#include <math/stat-utils.h>

#include "mathplot-log/utils.h"

namespace plt = matplotlibcpp;

namespace cider {
namespace mathplot {

std::vector<std::string> getMetrics() {
#ifdef ENG_NAMES
  return {"Compression, %", "Time Reduction, %", "Processing Time, %",
          "Retention Rate, %"};
#else
  return {"Коефіцієнт\nстиснення\n(CC), %",
          "Коефіцієнт\nзбереження\nчасу\n(ECR), %",
          "Коефіцієнт\nзбереження\nпокриття\n(RC), %"};
#endif
}

bool serialize(const RadarData& obj, serialization::Serializer& serializer) {
  serializer << obj.compressionCoefficients;
  serializer << obj.timeReductions;
  serializer << obj.processingTimes;
  serializer << obj.branchCoverage;
  return true;
}

bool deserialize(RadarData& obj,
                 const serialization::Deserializer& deserializer) {
  deserializer >> obj.compressionCoefficients;
  deserializer >> obj.timeReductions;
  deserializer >> obj.processingTimes;
  deserializer >> obj.branchCoverage;
  return true;
}

EfficiencyRadarPlot::EfficiencyRadarPlot(const std::string& logDir,
                                         const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

EfficiencyRadarPlot::~EfficiencyRadarPlot() {}

void EfficiencyRadarPlot::logCompression(const std::string& method,
                                         double coeff) {
  _radarData[method].compressionCoefficients.push_back(coeff);
}

void EfficiencyRadarPlot::logTimeReduction(const std::string& method,
                                           double reductionRate) {
  _radarData[method].timeReductions.push_back(reductionRate);
}

void EfficiencyRadarPlot::logProcessingTime(const std::string& method,
                                            double processingRate) {
  _radarData[method].processingTimes.push_back(processingRate);
}

void EfficiencyRadarPlot::logCoverage(const std::string& method,
                                      double branchCoverage) {
  _radarData[method].branchCoverage.push_back(branchCoverage);
}

void EfficiencyRadarPlot::serialize(const std::string& filePath) {
  try {
    serialization::Serializer serializer;
    serializer << _radarData;
    serializer << _order;
    serializer << _originalCoverage;
    serializer.save(filePath);
  } catch (...) {
    tlog_info << "Radar serialization failed: " << filePath << std::endl;
  }
}

bool EfficiencyRadarPlot::load() {
  try {
    serialization::Deserializer deserializer(ensureExtension(m_path, ".bin"));
    deserializer >> _radarData;
    deserializer >> _order;
    deserializer >> _originalCoverage;
  } catch (const std::exception& e) {
    tlog_info << "Radar load failed: " << e.what() << std::endl;
    return false;
  }
  return true;
}

void EfficiencyRadarPlot::plot() {
  plt::clf();

  if (_order.empty() || _radarData.empty()) {
    tlog_info << "No data for radar plot\n";
    return;
  }

  tlog_info << "Radar plot path: " << m_path << std::endl;

  struct NormVals final {
    double CR, ERC, CC, RC;
  };
  std::unordered_map<std::string, NormVals> avgVals;

  for (auto method : _order) {
    const auto it = _radarData.find(method);
    if (it == _radarData.end())
      continue;

    const auto& d = it->second;

    avgVals[method] = {math_stat::mean(d.compressionCoefficients),
                       math_stat::mean(d.timeReductions),
                       math_stat::mean(d.processingTimes),
                       math_stat::mean(d.branchCoverage)};
  }

  double minComp = 1e9, maxComp = 0, minTR = 1e9, maxTR = 0, minProc = 1e18,
         maxProc = 0, minBranchCov = 1e9, maxBranchCov = 0;

  for (auto& kv : avgVals) {
    minComp = 0;
    maxComp = std::max(maxComp, kv.second.CR);
    minTR = 0;
    maxTR = std::max(maxTR, kv.second.ERC);
    minProc = 0;
    maxProc = std::max(maxProc, kv.second.CC);
    minBranchCov = 0;
    maxBranchCov = std::max(maxBranchCov, kv.second.RC);
  }

  const auto& metrics = getMetrics();
  size_t numVars = metrics.size();
  std::vector<double> angles(numVars + 1);

  for (size_t i = 0; i < numVars; i++) {
    angles[i] = 2 * M_PI * i / numVars + M_PI / 2;
  }
  angles[numVars] = angles[0];

  for (auto& method : _order) {
    const auto it = avgVals.find(method);
    if (it == avgVals.end())
      continue;

    const auto& v = it->second;
    std::vector<double> values = {v.CR, v.ERC, v.RC / _originalCoverage};

    tlog_info << "Method: " << method << std::endl;
    tlog_info << "CC: " << values[0] << std::endl;
    tlog_info << "ECR: " << values[1] << std::endl;
    tlog_info << "RC: " << values[2] << std::endl;

    values.push_back(values[0]);

    std::vector<double> xs(values.size()), ys(values.size());
    for (size_t i = 0; i < values.size(); i++) {
      xs[i] = values[i] * cos(angles[i]);
      ys[i] = values[i] * sin(angles[i]);
    }

    plt::plot(xs, ys, {{"linewidth", "0.2"}});
    plt::fill(xs, ys, {{"label", method}}, 0.3);
  }

  plt::text(0.80, -0.35, metrics[2], {{"fontsize", "14"}});
  plt::text(-0.6, 0.95, metrics[0], {{"fontsize", "14"}});
  plt::text(-1.25, -0.35, metrics[1], {{"fontsize", "14"}});

  std::vector<double> levels = {0.0, 0.2, 0.4, 0.6, 0.8, 1.0};
  for (double r : levels) {
    std::vector<double> xs(numVars + 1), ys(numVars + 1);
    for (size_t i = 0; i < numVars; i++) {
      xs[i] = r * cos(angles[i]);
      ys[i] = r * sin(angles[i]);
    }
    xs[numVars] = xs[0];
    ys[numVars] = ys[0];

    plt::plot(xs, ys,
              {{"color", "gray"}, {"linestyle", "--"}, {"linewidth", "0.2"}});

    std::ostringstream oss;
    oss << std::setprecision(3) << (r * 100);

    const auto cordX = -0.05;
    const auto cordY = r + 0.01;
    plt::text(cordX, cordY, oss.str(),
              {{"ha", "left"}, {"va", "center"}, {"fontsize", "8"}});
  }

  disableFrame();

  plt::legend();
  plt::axis("equal");

  plt::save(ensureExtension(m_path, ".png"), 1200);
  plt::close();

  serialize(ensureExtension(m_path, ".bin"));
}

}  // namespace mathplot
}  // namespace cider
