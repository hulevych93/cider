#include "stat-utils.h"

namespace cider {
namespace math_stat {

double normalize(double value, double minVal, double maxVal, bool inverse) {
  if (std::fabs(maxVal - minVal) < 1e-12)
    return 0.0;
  double norm = (value - minVal) / (maxVal - minVal);
  return inverse ? 1.0 - norm : norm;
}

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

void computeMeanAndStd(const std::vector<std::vector<double>>& values,
                       std::vector<double>& meanOut,
                       std::vector<double>& stdOut) {
  size_t maxLen = 0;
  for (const auto& vec : values) {
    maxLen = std::max(maxLen, vec.size());
  }

  meanOut.resize(maxLen, 0.0);
  stdOut.resize(maxLen, 0.0);

  for (size_t i = 0; i < maxLen; ++i) {
    std::vector<double> column;
    for (const auto& vec : values) {
      if (i < vec.size()) {
        column.push_back(vec[i]);
      }
    }

    if (!column.empty()) {
      double avg = mean(column);
      meanOut[i] = avg;
      stdOut[i] = stddev(column, avg);
    } else {
      meanOut[i] = 0.0;
      stdOut[i] = 0.0;
    }
  }

  for (size_t i = 1; i < meanOut.size(); ++i) {
    if (meanOut[i] < meanOut[i - 1]) {
      meanOut[i] = meanOut[i - 1];
    }
  }
}

}  // namespace math_stat
}  // namespace cider
