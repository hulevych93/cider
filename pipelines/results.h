// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT
#pragma once

#include "coverage/coverage.h"

#include "coverage/cfg_coverage.h"
#include "coverage/gcov_coverage.h"

#include "recorder/details/action.h"
#include "recorder/details/params.h"

#include "serialization/deserializer.h"
#include "serialization/serializer.h"

#include <unordered_map>

namespace cider {
namespace pipelines {

struct Input final {
  recorder::Actions actions;
  std::string testOrLibName;
};

struct Result final : serialization::SerializableTag {
  std::string testCaseName;
  recorder::Actions oldActions;
  recorder::Actions newActions;
  gcov_coverage::CoverageReport oldReport;
  gcov_coverage::CoverageReport newReport;
  cfg_coverage::Coverage oldCfgReport;
  cfg_coverage::Coverage newCgfReport;
  unsigned long timeElapsedMcs = 0;
  unsigned long oldExecutionTimeMcs = 0;
  unsigned long newExecutionTimeMcs = 0;
  std::optional<unsigned long> coverageReachedLength;
};

struct MethodResults final : serialization::SerializableTag {
  std::vector<Result> entries;
  unsigned long totalTimeElapsedMcs = 0;
  unsigned long failedCount = 0;
  unsigned long sessionsCount = 0;
  unsigned long coverageReachedCount = 0;
};

double getMinimizationEfficency(const Result& result);

using Results = std::unordered_map<std::string, MethodResults>;

bool serialize(const Result& obj, serialization::Serializer& serializer);
bool deserialize(Result& obj, const serialization::Deserializer& deserializer);

bool serialize(const MethodResults& obj, serialization::Serializer& serializer);
bool deserialize(MethodResults& obj,
                 const serialization::Deserializer& deserializer);

void printResultsSummary(const std::string& method, const Results& results);
void printResultsSummary(const Results& results);
void printResult(const Result& result);

class RobustMedianFilter {
 public:
  RobustMedianFilter(size_t window = 50, double k = 3.5)
      : window_(window), k_(k) {}

  bool accept(long long timeMcs) {
    auto& buf = buffer_;

    std::vector<long long> sample(buf.begin(), buf.end());
    sample.push_back(timeMcs);

    if (sample.size() < 5) {
      push(buf, timeMcs);
      return true;
    }

    double med = median(sample);
    std::vector<double> dev(sample.size());
    std::transform(sample.begin(), sample.end(), dev.begin(),
                   [med](long long v) { return std::abs(double(v) - med); });
    double mad = median(dev);

    double tol = (mad > 0.0) ? k_ * mad : std::max(1.0, 0.01 * med);
    bool ok = std::abs(double(timeMcs) - med) <= tol;

    push(buf, timeMcs);
    return ok;
  }

 private:
  std::deque<long long> buffer_;
  size_t window_;
  double k_;

  template <class T>
  static double median(std::vector<T>& a) {
    size_t n = a.size();
    size_t mid = n / 2;
    std::nth_element(a.begin(), a.begin() + mid, a.end());
    if (n % 2 == 1)
      return double(a[mid]);
    auto lo = *std::max_element(a.begin(), a.begin() + mid);
    return 0.5 * (double(lo) + double(a[mid]));
  }

  static double median(const std::vector<double>& x) {
    std::vector<double> a = x;
    return median(a);
  }

  void push(std::deque<long long>& buf, long long v) {
    buf.push_back(v);
    if (buf.size() > window_)
      buf.pop_front();
  }
};

struct MethodTestKey {
  std::string method;
  std::string test;

  bool operator==(const MethodTestKey& other) const {
    return method == other.method && test == other.test;
  }
};

struct MethodTestKeyHash {
  std::size_t operator()(const MethodTestKey& k) const {
    return std::hash<std::string>()(k.method + "#" + k.test);
  }
};

class FilterManager {
 public:
  explicit FilterManager(size_t window = 50, double k = 3.5)
      : window_(window), k_(k) {}

  bool accept(const std::string& methodName,
              const std::string& testName,
              long long timeMcs) {
    MethodTestKey key{methodName, testName};
    auto& filter = filters_[key];
    if (!filter) {
      filter = std::make_unique<RobustMedianFilter>(window_, k_);
    }
    return filter->accept(timeMcs);
  }

 private:
  size_t window_;
  double k_;
  std::unordered_map<MethodTestKey,
                     std::unique_ptr<RobustMedianFilter>,
                     MethodTestKeyHash>
      filters_;
};

}  // namespace pipelines
}  // namespace cider
