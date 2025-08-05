#include "metrics.h"

#include <iomanip>  // для std::setprecision
#include <iostream>

namespace cider {
namespace pipelines {

double computeStdDev(const std::vector<double>& values, double mean) {
  if (values.size() <= 1)
    return 0.0;
  double sum = 0;
  for (double v : values)
    sum += (v - mean) * (v - mean);
  return std::sqrt(sum / (values.size() - 1));
}

Metrics computeMetrics(const std::vector<Result>& results) {
  Metrics m;
  size_t n = 0;

  std::vector<double> oldCovs, newCovs, oldCfgs, newCfgs, oldLens, newLens,
      times;

  std::cout << "=== computeMetrics START ===\n";

  for (size_t i = 0; i < results.size(); ++i) {
    const auto& r = results[i];

    if (r.oldReport.branchCov.percent == 0 ||
        r.oldCfgReport.getPercentage() == 0) {
      continue;
    }

    std::cout << "\nResult #" << i + 1 << " (" << r.testCaseName << "):\n";
    std::cout << "  OldCov: " << r.oldReport.branchCov.percent
              << ", NewCov: " << r.newReport.branchCov.percent << "\n";
    std::cout << "  OldCFG: " << r.oldCfgReport.getPercentage()
              << ", NewCFG: " << r.newCgfReport.getPercentage() << "\n";
    std::cout << "  OldLen: " << r.oldActions.size()
              << ", NewLen: " << r.newActions.size() << "\n";
    std::cout << "  Time(ms): " << r.timeElapsedMs << "\n";

    ++n;
    oldCovs.push_back(r.oldReport.branchCov.percent);
    newCovs.push_back(r.newReport.branchCov.percent);
    oldCfgs.push_back(r.oldCfgReport.getPercentage());
    newCfgs.push_back(r.newCgfReport.getPercentage());
    oldLens.push_back(r.oldActions.size());
    newLens.push_back(r.newActions.size());
    times.push_back(r.timeElapsedMs);
  }

  std::cout << "Total results: " << n << "\n";

  if (n > 0) {
    auto mean = [](const std::vector<double>& v) {
      return std::accumulate(v.begin(), v.end(), 0.0) / v.size();
    };

    m.avgOldCov = mean(oldCovs);
    m.stdOldCov = computeStdDev(oldCovs, m.avgOldCov);
    m.avgNewCov = mean(newCovs);
    m.stdNewCov = computeStdDev(newCovs, m.avgNewCov);
    m.avgOldCfg = mean(oldCfgs);
    m.stdOldCfg = computeStdDev(oldCfgs, m.avgOldCfg);
    m.avgNewCfg = mean(newCfgs);
    m.stdNewCfg = computeStdDev(newCfgs, m.avgNewCfg);
    m.avgOldLen = mean(oldLens);
    m.stdOldLen = computeStdDev(oldLens, m.avgOldLen);
    m.avgNewLen = mean(newLens);
    m.stdNewLen = computeStdDev(newLens, m.avgNewLen);
    m.avgTime = mean(times);
    m.stdTime = computeStdDev(times, m.avgTime);

    m.compression = (m.avgOldLen - m.avgNewLen) / m.avgOldLen;
    m.covDelta = m.avgNewCov - m.avgOldCov;
    m.cfgDelta = m.avgNewCfg - m.avgOldCfg;

    m.effScore = m.covDelta + m.compression - 0.1 * (m.avgTime / 1000.0) -
                 0.5 * std::abs(m.cfgDelta);
  }

  std::cout << "=== computeMetrics END ===\n";
  return m;
}
}  // namespace pipelines
}  // namespace cider
