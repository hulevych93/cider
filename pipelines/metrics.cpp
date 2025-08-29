#include "metrics.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include <iomanip>  // для std::setprecision
#include <iostream>

namespace cider {
namespace pipelines {

namespace {
double computeVariance(const std::vector<double>& values, double mean) {
  if (values.empty())
    return 0.0;
  double sum = 0;
  for (double v : values)
    sum += (v - mean) * (v - mean);
  return sum / values.size();  // population variance
}

double computeStdDev(const std::vector<double>& values, double mean) {
  if (values.size() <= 1)
    return 0.0;
  double sum = 0;
  for (double v : values)
    sum += (v - mean) * (v - mean);
  return std::sqrt(sum / (values.size() - 1));
}

unsigned long getCovReachLength(const Result& result) {
  if (result.coverageReachedLength.has_value()) {
    return result.coverageReachedLength.value();
  } else {
    return result.newActions.size();
  }
}

}  // namespace

Metrics computeMetrics(const std::vector<Result>& results) {
  Metrics m;
  size_t n = 0;

  std::vector<double> oldCovs, newCovs, oldCfgs, newCfgs, oldLens, newLens,
      covReachLens, times;

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
              << ", NewLen: " << r.newActions.size()
              << ", CovReachLen: " << getCovReachLength(r) << "\n";
    std::cout << "  Time(mcs): " << r.timeElapsedMcs << "\n";

    ++n;
    oldCovs.push_back(r.oldReport.branchCov.percent);
    newCovs.push_back(r.newReport.branchCov.percent);
    oldCfgs.push_back(r.oldCfgReport.getPercentage());
    newCfgs.push_back(r.newCgfReport.getPercentage());
    oldLens.push_back(r.oldActions.size());
    newLens.push_back(r.newActions.size());
    covReachLens.push_back(getCovReachLength(r));
    times.push_back(r.timeElapsedMcs);
  }

  std::cout << "Total results: " << n << "\n";

  if (n > 0) {
    auto mean = [](const std::vector<double>& v) {
      return std::accumulate(v.begin(), v.end(), 0.0) / v.size();
    };

    m.avgOldCov = mean(oldCovs);
    m.stdOldCov = computeStdDev(oldCovs, m.avgOldCov);
    m.varOldCov = computeVariance(oldCovs, m.avgOldCov);

    m.avgNewCov = mean(newCovs);
    m.stdNewCov = computeStdDev(newCovs, m.avgNewCov);
    m.varNewCov = computeVariance(newCovs, m.avgNewCov);

    m.avgOldCfg = mean(oldCfgs);
    m.stdOldCfg = computeStdDev(oldCfgs, m.avgOldCfg);
    m.varOldCfg = computeVariance(oldCfgs, m.avgOldCfg);

    m.avgNewCfg = mean(newCfgs);
    m.stdNewCfg = computeStdDev(newCfgs, m.avgNewCfg);
    m.varNewCfg = computeVariance(newCfgs, m.avgNewCfg);

    m.avgOldLen = mean(oldLens);
    m.stdOldLen = computeStdDev(oldLens, m.avgOldLen);
    m.varOldLen = computeVariance(oldLens, m.avgOldLen);

    m.avgNewLen = mean(newLens);
    m.stdNewLen = computeStdDev(newLens, m.avgNewLen);
    m.varNewLen = computeVariance(newLens, m.avgNewLen);

    m.avgCovReachLen = mean(covReachLens);
    m.stdCovReachLen = computeStdDev(covReachLens, m.avgCovReachLen);
    m.varCovReachLen = computeVariance(covReachLens, m.avgCovReachLen);

    m.avgTime = mean(times);
    m.stdTime = computeStdDev(times, m.avgTime);
    m.varTime = computeVariance(times, m.avgTime);

    m.compression = (m.avgOldLen - m.avgCovReachLen) / m.avgOldLen;
    m.covDelta = m.avgNewCov - m.avgOldCov;
    m.cfgDelta = m.avgNewCfg - m.avgOldCfg;
  }

  std::cout << "=== computeMetrics END ===\n";
  return m;
}

std::optional<size_t> computeCoverageReachedLength(
    const recorder::Actions& oldActions,
    const recorder::Actions& newActions,
    const std::string& libName,
    const cider::Cmd& cmd) {
  cider::gcov_coverage::CoverageMeasurment gcov_measurer{cmd, libName.c_str()};

  // Coverage для старих дій
  auto oldGcov = gcov_measurer.getReport(oldActions);
  if (!oldGcov.has_value()) {
    std::cout << "[ERROR] Old coverage not available\n";
    return std::nullopt;
  }
  const size_t oldCovered = oldGcov->report.branchCov.covered;
  std::cout << "[INFO] Old covered branches = " << oldCovered << "\n";

  // Coverage для всіх нових дій
  auto fullNewGcov = gcov_measurer.getReport(newActions);
  if (!fullNewGcov.has_value()) {
    std::cout << "[ERROR] Full new coverage not available\n";
    return std::nullopt;
  }
  const size_t newCovered = fullNewGcov->report.branchCov.covered;
  std::cout << "[INFO] Full new covered branches = " << newCovered
            << " (actions = " << newActions.size() << ")\n";

  // Якщо навіть повний сценарій не досяг старого покриття → фейл
  if (newCovered < oldCovered) {
    std::cout << "[WARN] New coverage (" << newCovered << ") < old coverage ("
              << oldCovered << ") → skip binary search\n";
    return std::nullopt;
  }

  // Бінарний пошук
  size_t left = 1;
  size_t right = newActions.size();
  size_t answer = right;

  std::cout << "[INFO] Start binary search in range [1, " << right << "]\n";

  int step = 0;
  while (left <= right) {
    size_t mid = (left + right) / 2;
    step++;

    recorder::Actions prefix(newActions.begin(), newActions.begin() + mid);
    auto midGcov = gcov_measurer.getReport(prefix);

    if (!midGcov.has_value()) {
      std::cout << "[ERROR] Coverage failed at length " << mid << " (step "
                << step << ")\n";
      return std::nullopt;
    }

    size_t midCovered = midGcov->report.branchCov.covered;

    std::cout << "  [STEP " << step << "] mid=" << mid
              << " → covered=" << midCovered << " (range=[" << left << ","
              << right << "])\n";

    if (midCovered >= oldCovered) {
      answer = mid;
      std::cout << "    ✓ Candidate found at " << mid
                << " (covered=" << midCovered << " >= " << oldCovered << ")\n";
      if (mid == 1)
        break;
      right = mid - 1;
    } else {
      std::cout << "    ✗ Too low, moving right\n";
      left = mid + 1;
    }
  }

  std::cout << "[INFO] Minimal length where coverage reached = " << answer
            << " / " << newActions.size() << "\n";

  return answer;
}

}  // namespace pipelines
}  // namespace cider
