// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <deque>
#include <numeric>
#include <string>
#include <vector>

namespace cider {
namespace math_stat {

template <typename Type>
double mean(const std::vector<Type>& vec) {
  if (vec.empty())
    return 0.0;

  // Use unsigned long long to safely hold the sum
  double sum = 0;
  for (const auto& item : vec) {
    sum += item;
  }

  return static_cast<double>(sum) / vec.size();
}

template <typename Type>
double variance(const std::vector<Type>& v, const double avg) {
  if (v.size() < 2)
    return 0.0;
  double sum = 0;
  for (double x : v)
    sum += (x - avg) * (x - avg);
  return sum / (v.size() - 1);
}

template <typename Type>
double stddev(const std::vector<Type>& v, const double avg) {
  return std::sqrt(variance(v, avg));
}

double normalize(double value,
                 double minVal,
                 double maxVal,
                 bool inverse = false);

struct BoxStats final {
  double q1, median, q3, iqr, mean = 0;
  double lower_whisker, upper_whisker;
  std::vector<double> outliers;
};

BoxStats compute_box(std::vector<double> data);

void computeMeanAndStd(const std::vector<std::vector<double>>& values,
                       std::vector<double>& meanOut,
                       std::vector<double>& stdOut);

class RobustMedianFilter {
 public:
  RobustMedianFilter(size_t window = 50, double k = 1.5)
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

}  // namespace math_stat
}  // namespace cider
