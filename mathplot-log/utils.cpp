// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "utils.h"

#include <algorithm>
#include <numeric>
#include <vector>

#include <filesystem>

namespace cider {

BoxStats compute_box(std::vector<double> data) {
  BoxStats s;
  if (data.empty())
    return s;

  std::sort(data.begin(), data.end());
  const size_t n = data.size();

  // Interpolated percentile
  auto percentile = [&](double p) -> double {
    double pos = p * (n - 1);
    size_t i0 = static_cast<size_t>(std::floor(pos));
    size_t i1 = std::min(i0 + 1, n - 1);
    double frac = pos - i0;
    return data[i0] + frac * (data[i1] - data[i0]);
  };

  s.q1 = percentile(0.25);
  s.median = percentile(0.5);
  s.q3 = percentile(0.75);
  s.iqr = s.q3 - s.q1;

  double lower_bound = s.q1 - 1.5 * s.iqr;
  double upper_bound = s.q3 + 1.5 * s.iqr;

  // Mean
  double sum = 0.0;
  for (double v : data)
    sum += v;
  s.mean = sum / static_cast<double>(n);

  // Whiskers are the most extreme values within the bounds
  for (double v : data) {
    if (v >= lower_bound) {
      s.lower_whisker = v;
      break;
    }
  }

  for (auto it = data.rbegin(); it != data.rend(); ++it) {
    if (*it <= upper_bound) {
      s.upper_whisker = *it;
      break;
    }
  }

  // Outliers are outside the whiskers
  for (double v : data) {
    if (v < s.lower_whisker || v > s.upper_whisker) {
      s.outliers.push_back(v);
    }
  }

  return s;
}

double compute_average(const std::vector<size_t>& vec) {
  if (vec.empty())
    return 0.0;

  // Use unsigned long long to safely hold the sum
  unsigned long long sum = std::accumulate(vec.begin(), vec.end(), 0ULL);

  return static_cast<double>(sum) / vec.size();
}

void computeMeanAndStd(const std::vector<std::vector<double>>& values,
                       std::vector<double>& meanOut,
                       std::vector<double>& stdOut) {
  size_t maxLen = 0;
  for (const auto& vec : values) {
    maxLen = std::max(maxLen, vec.size());
  }

  meanOut.resize(maxLen, 0.0);
  stdOut.resize(maxLen, 0.0);
  std::vector<size_t> counts(maxLen, 0);

  // Сума для середнього
  for (const auto& vec : values) {
    for (size_t i = 0; i < vec.size(); ++i) {
      meanOut[i] += vec[i];
      counts[i]++;
    }
  }
  for (size_t i = 0; i < maxLen; ++i) {
    if (counts[i] > 0)
      meanOut[i] /= counts[i];
  }

  // Сума квадратів відхилень
  for (const auto& vec : values) {
    for (size_t i = 0; i < vec.size(); ++i) {
      double diff = vec[i] - meanOut[i];
      stdOut[i] += diff * diff;
    }
  }
  for (size_t i = 0; i < maxLen; ++i) {
    if (counts[i] > 1)
      stdOut[i] = std::sqrt(stdOut[i] / (counts[i] - 1));
    else
      stdOut[i] = 0.0;  // немає std для одного значення
  }
}

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

}  // namespace cider
