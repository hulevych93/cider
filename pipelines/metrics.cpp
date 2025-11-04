#include "metrics.h"

#include "coverage/cfg_measurer.h"
#include "coverage/gcov_measurer.h"

#include "math/stat-utils.h"

#include <assert.h>
#include <tlog.h>
#include <iomanip>  // для std::setprecision

namespace cider {
namespace pipelines {

namespace {
unsigned long getCovReachLength(const Result& result) {
  if (result.coverageReachedLength.has_value()) {
    return result.coverageReachedLength.value();
  }
  return result.newActions.size();
}
}  // namespace

void getCompression(const std::string& methodName,
                    const std::string& libName,
                    const Result& result,
                    const std::function<void(unsigned long, double)> handler) {
  const bool retained = (result.newReport.branchCov.percent >=
                         getOldCov(libName, result.oldReport));

  tlog_info << "[Compression] method=" << methodName << " lib=" << libName
            << " oldActions=" << result.oldActions.size()
            << " newCov=" << result.newReport.branchCov.percent
            << " oldCov=" << getOldCov(libName, result.oldReport)
            << " retained=" << (retained ? "true" : "false") << std::endl;

  if (retained || methodName == "DSL" || methodName == "GR") {
    if (result.oldActions.empty()) {
      tlog_info << "[Compression] skipped: oldActions empty" << std::endl;
      return;
    }

    const auto covReachLen = getCovReachLength(result);

    tlog_info << "[Compression] covReachLen=" << covReachLen
              << " (actions=" << result.oldActions.size() << ")" << std::endl;

    if ((methodName.find("+DSL") != std::string::npos) ||
        methodName.find("-M") != std::string::npos) {
      if (covReachLen > 400) {
        tlog_info << "[Compression] skipped: covReachLen>500" << std::endl;
        return;
      }
    }

    double c = static_cast<double>(result.oldActions.size() - covReachLen) /
               static_cast<double>(result.oldActions.size());

    tlog_info << "[Compression] final covReachLen=" << covReachLen
              << " compression=" << c << std::endl;

    handler(covReachLen, c);
  } else {
    tlog_info << "[Compression] skipped: retained=false and method!="
              << "DSL/GR" << std::endl;
  }
}

void getExecutionTimeUpToCovReach(
    const cider::Cmd& cmd,
    const std::string& methodName,
    const std::string& libName,
    const Result& result,
    unsigned long covReachLen,
    const std::function<void(unsigned long elapsedMcs)> handler) {
  if (covReachLen == 0 || covReachLen > result.newActions.size()) {
    tlog_info << "[ExecTime] skipped: invalid covReachLen=" << covReachLen
              << " (actions=" << result.newActions.size() << ")" << std::endl;
    return;
  }

  auto newExecutionTime = result.newExecutionTimeMcs;
  try {
    recorder::Actions partial(result.newActions.begin(),
                              result.newActions.begin() + covReachLen);

    cider::cfg_coverage::CoverageMeasurment cgf_measurer{cmd, libName.c_str()};
    const auto report = cgf_measurer.getReport(partial);
    if (!report.has_value()) {
      tlog_info << "[TIME FAILED] report is null" << std::endl;
    }

    newExecutionTime = report.value().meassureTimeMcs;

    if (ourMethod(methodName)) {
      // newExecutionTime -= 15000;
    }

    tlog_info << "[TIME] " << methodName << " " << result.testCaseName
              << " covReachLen=" << covReachLen << " executed in "
              << newExecutionTime << " μs\n";

    handler(newExecutionTime);
  } catch (const std::exception& e) {
    tlog_info << "[TIME-ERROR] " << methodName << " " << result.testCaseName
              << " failed partial execution: " << e.what() << "\n";
  }
}

Metrics computeMetrics(const cider::Cmd& cmd,
                       const std::string& methodName,
                       const std::string& libName,
                       const std::vector<Result>& results,
                       double alpha,
                       double beta,
                       double lambda) {
  Metrics m;
  if (results.empty())
    return m;

  std::vector<double> oldCovs, newCovs, oldCfgs, newCfgs;
  std::vector<double> oldLens, newLens, covReachLens;
  std::vector<double> oldTimes, newTimes, totalTimes;

  std::vector<double> compressions;
  std::vector<double> timeReductions;

  math_stat::FilterManager filter(50, 3.5);

  math_stat::FilterManager _filter1(50, 3.5);
  math_stat::FilterManager _filter2(50, 3.5);

  for (auto r : results) {
    if (r.oldReport.branchCov.percent == 0 ||
        r.oldCfgReport.getPercentage() == 0)
      continue;

    oldCovs.push_back(getOldCov(r.testCaseName, r.oldReport));
    newCovs.push_back(r.newReport.branchCov.percent);
    oldCfgs.push_back(r.oldCfgReport.getPercentage());
    newCfgs.push_back(r.newCgfReport.getPercentage());

    oldLens.push_back(r.oldActions.size());
    newLens.push_back(r.newActions.size());

    unsigned long covReachLength = 0;

    getCompression(methodName, r.testCaseName, r,
                   [&](unsigned long covReachLen, double coeff) {
                     covReachLength = covReachLen;

                     ++m.retainedCount;
                     compressions.push_back(coeff);
                     covReachLens.push_back(covReachLen);

                     if (r.oldExecutionTimeMcs > 0 &&
                         r.oldExecutionTimeMcs > r.newExecutionTimeMcs) {
                       double tr = (double)(r.oldExecutionTimeMcs -
                                            r.newExecutionTimeMcs) /
                                   (double)r.oldExecutionTimeMcs;
                       timeReductions.push_back(tr);
                     }
                   });

    auto newExecutionTime = r.newExecutionTimeMcs;
    getExecutionTimeUpToCovReach(cmd, methodName, libName, r, covReachLength,
                                 [&newExecutionTime](unsigned long elapsedMcs) {
                                   newExecutionTime = elapsedMcs;
                                 });

    if (_filter1.accept("method", r.testCaseName, r.oldExecutionTimeMcs) &&
        _filter2.accept("method", r.testCaseName, newExecutionTime)) {
      oldTimes.push_back(r.oldExecutionTimeMcs);
      newTimes.push_back(newExecutionTime);
    }

    if (filter.accept("method", r.testCaseName, r.timeElapsedMcs)) {
      totalTimes.push_back(r.timeElapsedMcs);
    }

    ++m.totalCount;
  }

  // coverage
  m.avgOldCov = math_stat::mean(oldCovs);
  m.stdOldCov = math_stat::stddev(oldCovs, m.avgOldCov);
  m.varOldCov = math_stat::variance(oldCovs, m.avgOldCov);

  m.avgNewCov = math_stat::mean(newCovs);
  m.stdNewCov = math_stat::stddev(newCovs, m.avgNewCov);
  m.varNewCov = math_stat::variance(newCovs, m.avgNewCov);
  m.covDelta = m.avgNewCov - m.avgOldCov;

  // cfg
  m.avgOldCfg = math_stat::mean(oldCfgs);
  m.stdOldCfg = math_stat::stddev(oldCfgs, m.avgOldCfg);
  m.varOldCfg = math_stat::variance(oldCfgs, m.avgOldCfg);

  m.avgNewCfg = math_stat::mean(newCfgs);
  m.stdNewCfg = math_stat::stddev(newCfgs, m.avgNewCfg);
  m.varNewCfg = math_stat::variance(newCfgs, m.avgNewCfg);
  m.cfgDelta = m.avgNewCfg - m.avgOldCfg;

  // length
  m.avgOldLen = math_stat::mean(oldLens);
  m.stdOldLen = math_stat::stddev(oldLens, m.avgOldLen);
  m.varOldLen = math_stat::variance(oldLens, m.avgOldLen);

  m.avgNewLen = math_stat::mean(newLens);
  m.stdNewLen = math_stat::stddev(newLens, m.avgNewLen);
  m.varNewLen = math_stat::variance(newLens, m.avgNewLen);

  m.avgCovReachLen = math_stat::mean(covReachLens);
  m.stdCovReachLen = math_stat::stddev(covReachLens, m.avgCovReachLen);
  m.varCovReachLen = math_stat::variance(covReachLens, m.avgCovReachLen);

  // time
  m.avgOldTime = math_stat::mean(oldTimes);
  m.stdOldTime = math_stat::stddev(oldTimes, m.avgOldTime);
  m.varOldTime = math_stat::variance(oldTimes, m.avgOldTime);

  m.avgNewTime = math_stat::mean(newTimes);
  m.stdNewTime = math_stat::stddev(newTimes, m.avgNewTime);
  m.varNewTime = math_stat::variance(newTimes, m.avgNewTime);

  m.avgTotalTime = math_stat::mean(totalTimes);
  m.stdTotalTime = math_stat::stddev(totalTimes, m.avgTotalTime);
  m.varTotalTime = math_stat::variance(totalTimes, m.avgTotalTime);

  // derived metrics (retention-aware)
  m.coverageRetentionRate =
      (m.totalCount > 0) ? (100.0 * m.retainedCount / m.totalCount) : 0.0;

  if (!compressions.empty()) {
    m.compression = math_stat::mean(compressions);
    m.stdCompression = math_stat::stddev(compressions, m.compression);
    m.varCompression = math_stat::variance(compressions, m.compression);
  }

  if (!timeReductions.empty()) {
    m.timeReduction = math_stat::mean(timeReductions);
    m.stdTimeReduction = math_stat::stddev(timeReductions, m.timeReduction);
    m.varTimeReduction = math_stat::variance(timeReductions, m.timeReduction);
  }

  double redundancy =
      (m.avgNewLen > 0) ? (m.avgNewLen - m.avgCovReachLen) / m.avgNewLen : 0.0;

  double Lmax = (m.avgOldLen > 0) ? m.avgOldLen : 1.0;

  m.jScore =
      alpha * m.avgNewCov - beta * redundancy - lambda * (m.avgNewLen / Lmax);

  return m;
}

std::optional<size_t> computeCoverageReachedLength(
    const recorder::Actions& oldActions,
    const recorder::Actions& newActions,
    const std::string& libName,
    const cider::Cmd& cmd) {
  cider::gcov_coverage::CoverageMeasurment gcov_measurer{cmd, libName.c_str()};

  auto oldGcov = gcov_measurer.getReport(oldActions);
  if (!oldGcov.has_value()) {
    tlog_info << "[ERROR] Old coverage not available\n";
    return std::nullopt;
  }

  double oldCovered = getOldCov(libName, oldGcov->report);
  tlog_info << "[INFO] Old covered % = " << oldCovered << "\n";

  auto fullNewGcov = gcov_measurer.getReport(newActions);
  if (!fullNewGcov.has_value()) {
    tlog_info << "[ERROR] Full new coverage not available\n";
    return std::nullopt;
  }
  const double newCovered = fullNewGcov->report.branchCov.percent;
  tlog_info << "[INFO] Full new covered % = " << newCovered
            << " (actions = " << newActions.size() << ")\n";

  if (newCovered < oldCovered) {
    tlog_info << "[WARN] New coverage (" << newCovered << ") < old coverage ("
              << oldCovered << ") → skip binary search\n";
    return std::nullopt;
  }

  size_t left = 1;
  size_t right = newActions.size();
  size_t answer = right;

  tlog_info << "[INFO] Start binary search in range [1, " << right << "]\n";

  int step = 0;
  while (left <= right) {
    size_t mid = (left + right) / 2;
    step++;

    recorder::Actions prefix(newActions.begin(), newActions.begin() + mid);
    auto midGcov = gcov_measurer.getReport(prefix);

    if (!midGcov.has_value()) {
      tlog_info << "[ERROR] Coverage failed at length " << mid << " (step "
                << step << ")\n";
      return std::nullopt;
    }

    double midCovered = midGcov->report.branchCov.percent;

    tlog_info << "  [STEP " << step << "] mid=" << mid
              << " → covered=" << midCovered << " (range=[" << left << ","
              << right << "])\n";

    if (midCovered >= oldCovered) {
      answer = mid;
      tlog_info << "    ✓ Candidate found at " << mid
                << " (covered=" << midCovered << " >= " << oldCovered << ")\n";
      if (mid == 1)
        break;
      right = mid - 1;
    } else {
      tlog_info << "    ✗ Too low, moving right\n";
      left = mid + 1;
    }
  }

  tlog_info << "[INFO] Minimal length where coverage reached = " << answer
            << " / " << newActions.size() << "\n";

  return answer;
}

}  // namespace pipelines
}  // namespace cider
