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

struct ExecTimesData final : serialization::SerializableTag {
  std::vector<size_t> oldTimes;
  std::vector<size_t> newTimes;
};

bool serialize(const ExecTimesData& obj, serialization::Serializer& serializer);
bool deserialize(ExecTimesData& obj,
                 const serialization::Deserializer& deserializer);

class ExecTimesBarPlot final : public IBasicPlot {
 public:
  ExecTimesBarPlot(const std::string& logDir, const std::string& logFileName);
  ~ExecTimesBarPlot() override;

  void setOrder(const std::vector<std::string>& order) override {
    _order = order;
  }

  void log(const std::string& method, size_t oldTime, size_t newTime);

  void serialize(const std::string& filePath) override;

  bool load() override;

  void plot() override;

 private:
  std::unordered_map<std::string, ExecTimesData> _barData;
  std::vector<std::string> _order;

  std::string m_path;
};

}  // namespace mathplot
}  // namespace cider
