#include "metrics.h"

namespace cider {
namespace pipelines {

Metrics computeMetrics(const std::vector<Result>& results) {
  Metrics m;
  size_t n = results.size();

  for (const auto& r : results) {
    m.avgOldCov += r.oldReport.branchCov.percent;
    m.avgNewCov += r.newReport.branchCov.percent;

    m.avgOldCfg += r.oldCfgReport.getPercentage();
    m.avgNewCfg += r.newCgfReport.getPercentage();

    m.avgOldLen += r.oldActions.size();
    m.avgNewLen += r.newActions.size();

    m.avgTime += r.timeElapsedMs;
  }

  if (n > 0) {
    m.avgOldCov /= n;
    m.avgNewCov /= n;
    m.avgOldCfg /= n;
    m.avgNewCfg /= n;
    m.avgOldLen /= n;
    m.avgNewLen /= n;
    m.avgTime /= n;

    m.compression = (m.avgOldLen - m.avgNewLen) / m.avgOldLen;
    m.covDelta = m.avgNewCov - m.avgOldCov;
    m.cfgDelta = m.avgNewCfg - m.avgOldCfg;

    m.effScore = (m.covDelta + m.compression - 0.1 * (m.avgTime / 1000.0) -
                  0.5 * std::abs(m.cfgDelta));
  }

  return m;
}

}  // namespace pipelines
}  // namespace cider
