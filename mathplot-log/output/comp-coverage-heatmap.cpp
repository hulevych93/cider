// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "comp-coverage-heatmap.h"

#include <fmt/format.h>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>

#include <matplotlibcpp.h>
namespace plt = matplotlibcpp;

#include "mathplot-log/utils.h"

namespace cider {
namespace mathplot {

CoverageHeatmapPlot::CoverageHeatmapPlot(const std::string& logDir,
                                         const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

CoverageHeatmapPlot::~CoverageHeatmapPlot() {
  plt::save(ensurePngExtension(m_path), 1200);
  plt::close();
}

void CoverageHeatmapPlot::add(const std::string& methodLabel,
                              const std::vector<uint8_t>& coveredBranches) {
  m_rawMatrix[methodLabel].push_back(coveredBranches);
}

void CoverageHeatmapPlot::plot() {
  plt::clf();

  if (m_rawMatrix.empty() || m_original.empty())
    return;

  std::vector<std::string> methodNames;
  size_t totalBranches = 0;

  for (auto iter = m_rawMatrix.begin(); iter != m_rawMatrix.end(); ++iter) {
    const auto& rows = iter->second;
    if (!rows.empty()) {
      totalBranches = rows[0].size();
      break;
    }
  }

  if (m_original.size() != totalBranches)
    throw std::runtime_error("Baseline coverage mask size mismatch");

  std::vector<std::vector<double>> Z_filtered;

  for (auto iter = m_rawMatrix.begin(); iter != m_rawMatrix.end(); ++iter) {
    const std::string& method = iter->first;
    const auto& rows = iter->second;

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
      if (m_original[i])
        Z_filtered.back().push_back(avg[i]);
    }
  }

  const size_t methodsCount = Z_filtered.size();

  std::vector<double> y(methodsCount);
  for (size_t i = 0; i < methodsCount; ++i)
    y[i] = i;

  auto img = plt::imshow_pylist(Z_filtered, {{"cmap", "Blues"},
                                             {"interpolation", "none"},
                                             {"vmin", "0.0"},
                                             {"vmax", "1.0"},
                                             {"aspect", "auto"}});
  plt::colorbar(img);

  applyPublicationStyle();
  plt::yticks(y, methodNames);
  // plt::xticks(xticks);
  plt::xlabel("Branch");
  plt::grid(false);

  plt::pause(0.01);
}

}  // namespace mathplot
}  // namespace cider
