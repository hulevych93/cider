// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

#include "mathplot-log/output/basic-plot.h"

#include "serialization/deserializer.h"
#include "serialization/serializable.h"
#include "serialization/serializer.h"

namespace cider {
namespace mathplot {

struct RadarData final : serialization::SerializableTag {
  std::vector<double> compressionCoefficients;
  std::vector<double> timeReductions;
  std::vector<double> processingTimes;
  std::vector<double> branchCoverage;
};

bool serialize(const RadarData& obj, serialization::Serializer& serializer);
bool deserialize(RadarData& obj,
                 const serialization::Deserializer& deserializer);

class EfficiencyRadarPlot final : public IBasicPlot {
 public:
  EfficiencyRadarPlot(const std::string& logDir,
                      const std::string& logFileName);
  ~EfficiencyRadarPlot() override;

  void setOrder(const std::vector<std::string>& order) override {
    _order = order;
  }

  void logCompression(const std::string& method, double coeff);
  void logTimeReduction(const std::string& method, double reductionRate);
  void logProcessingTime(const std::string& method, double processingRate);
  void logCoverage(const std::string& method, double branchCoverage);

  void serialize(const std::string& filePath) override;

  bool load() override;

  void plot() override;

 private:
  std::unordered_map<std::string, RadarData> _radarData;
  std::vector<std::string> _order;

  std::string m_path;
};

}  // namespace mathplot
}  // namespace cider
