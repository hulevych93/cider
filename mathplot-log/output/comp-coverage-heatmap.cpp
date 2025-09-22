// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "comp-coverage-heatmap.h"

#include <fmt/format.h>
#include <tlog.h>
#include <numeric>
#include <sstream>
#include <stdexcept>

#include <matplotlibcpp.h>
namespace plt = matplotlibcpp;

#include "mathplot-log/utils.h"

namespace cider {
namespace mathplot {

namespace {
std::string getYAxisName() {
#ifdef ENG_NAMES
  return "Method / configuration";
#else
  return "Метод / конфігурація";
#endif
}

std::string getXAxisName() {
#ifdef ENG_NAMES
  return "Basic Block IDs";
#else
  return "Ідентифікатор базового блоку програми, ум. од.";
#endif
}

std::string getOriginalTSName() {
#ifdef ENG_NAMES
  return "Original TS";
#else
  return "Оригінальний ТН";
#endif
}
}  // namespace

CoverageHeatmapPlot::CoverageHeatmapPlot(const std::string& logDir,
                                         const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

CoverageHeatmapPlot::~CoverageHeatmapPlot() {
  plt::save(ensureExtension(m_path, ".eps"), 1200);
  plt::close();
}

void CoverageHeatmapPlot::serialize(const std::string& filePath) {
  try {
    serialization::Serializer serializer;
    serializer << _rawMatrix;
    serializer << _order;
    serializer << _original;
    serializer.save(filePath);
  } catch (...) {
    tlog_info << "Graph serialization failed : " << filePath << std::endl;
  }
}

bool CoverageHeatmapPlot::load() {
  try {
    serialization::Deserializer deserializer(ensureExtension(m_path, ".bin"));
    deserializer >> _rawMatrix;
    deserializer >> _order;
    deserializer >> _original;
  } catch (const std::exception& e) {
    tlog_info << e.what() << std::endl;
    return false;
  }
  return true;
}

void CoverageHeatmapPlot::add(const std::string& methodLabel,
                              const std::vector<uint8_t>& coveredBranches) {
  _rawMatrix[methodLabel].push_back(coveredBranches);
}

void CoverageHeatmapPlot::plot() {
  plt::clf();

  if (_rawMatrix.empty() || _original.empty())
    return;

  std::vector<std::string> methodNames;
  size_t totalBranches = 0;

  for (auto iter = _rawMatrix.begin(); iter != _rawMatrix.end(); ++iter) {
    const auto& rows = iter->second;
    if (!rows.empty()) {
      totalBranches = rows[0].size();
      break;
    }
  }

  if (_original.size() != totalBranches)
    throw std::runtime_error("Baseline coverage mask size mismatch");

  std::vector<std::vector<double>> Z_filtered;

  for (const auto& orderName : _order) {
    const auto it = _rawMatrix.find(orderName);
    if (it == _rawMatrix.end()) {
      tlog_info << "Warning method not simulated: " << orderName << std::endl;
      continue;
    }

    const std::string& method = it->first;
    const auto& rows = it->second;

    methodNames.push_back(method);

    std::vector<double> avg(totalBranches, 0.0);
    for (const auto& row : rows) {
      if (row.size() != totalBranches)
        throw std::runtime_error("Inconsistent branch map size");
      for (size_t i = 0; i < totalBranches; ++i)
        avg[i] += row[i];
    }

    for (size_t i = 0; i < totalBranches; ++i)
      avg[i] /= rows.size();

    Z_filtered.emplace_back();
    for (size_t i = 0; i < totalBranches; ++i) {
      if (_original[i])
        Z_filtered.back().push_back(avg[i]);
    }
  }

  const size_t methodsCount = Z_filtered.size();

  std::vector<double> y(methodsCount);
  for (size_t i = 0; i < methodsCount; ++i)
    y[i] = i;

  auto img = plt::imshow_pylist(Z_filtered, {{"cmap", "magma"},
                                             {"interpolation", "none"},
                                             {"vmin", "0.0"},
                                             {"vmax", "1.0"},
                                             {"aspect", "auto"}});
  plt::colorbar(img);

  tightLighout();
  applyPublicationStyle();
  plt::yticks(y, methodNames, {{"fontsize", "7"}});

  plt::xlabel(getXAxisName());
  plt::ylabel(getYAxisName());
  plt::grid(false);

  plt::pause(0.01);

  serialize(ensureExtension(m_path, ".bin"));
}

}  // namespace mathplot
}  // namespace cider
