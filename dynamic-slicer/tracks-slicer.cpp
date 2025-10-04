// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "dynamic-slicer.h"

#include <tlog.h>
#include <algorithm>
#include <numeric>
#include <string>
#include <unordered_set>
#include <vector>

namespace cider {
namespace dslicer {

std::vector<recorder::Action> run_dd_basic_blocks(
    const BatchTracksDSlicingSettings& settings,
    const std::vector<recorder::Action>& actionSpace) {
  auto currentSpace = deepCopy(actionSpace);

  double baseline = settings.baseline;
  if (baseline <= std::numeric_limits<double>::epsilon()) {
    baseline = settings.objFunc(currentSpace).coverage;
  }

  const auto curCov = settings.objFunc(currentSpace);
  if (curCov.coverage + 1e-9 < baseline) {
    return currentSpace;
  }

  FineObjectiveValue fineRes = settings.fineObjFunc(currentSpace);
  if (fineRes.fineCoveredTracks.empty())
    return currentSpace;

  size_t trackCount = 0;
  for (const auto& v : fineRes.fineCoveredTracks) {
    if (!v.empty()) {
      trackCount = v.size();
      break;
    }
  }
  if (trackCount == 0) {
    tlog_info << "[Slice-Batch-Trace] No coverage tracks found\n";
    return currentSpace;
  }

  struct Step final {
    recorder::Action action;
    std::vector<std::uint8_t> coverage;
  };

  std::vector<Step> steps;
  steps.reserve(currentSpace.size());
  for (size_t i = 0; i < currentSpace.size(); ++i) {
    std::vector<std::uint8_t> cov = fineRes.fineCoveredTracks[i];
    if (cov.empty())
      cov.assign(trackCount, 0);
    steps.push_back({currentSpace[i], std::move(cov)});
  }

  tlog_info << "[Slice-Batch-Trace] Original length=" << steps.size()
            << " tracks=" << trackCount << " baseline=" << baseline << "\n";

  std::vector<int> freq(trackCount, 0);
  for (const auto& s : steps)
    for (size_t j = 0; j < trackCount; ++j)
      if (s.coverage[j])
        ++freq[j];

  const size_t batchSize = std::max<size_t>(1, 30);
  size_t removedTotal = 0;

  std::vector<Step> snapSteps;
  std::vector<int> snapFreq;
  size_t snapI = 0;
  bool hasSnapshot = false;
  size_t pendingInBatch = 0;

  auto takeSnapshot = [&](size_t i) {
    snapSteps = steps;
    snapFreq = freq;
    snapI = i;
    hasSnapshot = true;
    pendingInBatch = 0;
  };

  auto buildScenario = [&]() {
    std::vector<recorder::Action> out;
    out.reserve(steps.size());
    for (auto& s : steps)
      out.push_back(s.action);
    return out;
  };

  size_t i = 0;
  while (i < steps.size()) {
    if (!hasSnapshot)
      takeSnapshot(i);

    const auto& cov = steps[i].coverage;
    bool removable = true;
    int uniq = 0, dup = 0;

    for (size_t j = 0; j < trackCount; ++j) {
      if (!cov[j])
        continue;
      if (freq[j] == 1) {
        removable = false;
        ++uniq;
      } else {
        ++dup;
      }
    }

    if (removable) {
      for (size_t j = 0; j < trackCount; ++j)
        if (cov[j])
          --freq[j];

      tlog_info << "  [Slice-Batch-Trace] Removing step " << i
                << " (uniq=" << uniq << ", dup=" << dup << ")\n";

      steps.erase(steps.begin() + i);
      ++pendingInBatch;

      if (pendingInBatch >= batchSize || i >= steps.size()) {
        auto scenario = buildScenario();
        auto newCov = settings.objFunc(scenario);
        tlog_info << "  [Check@" << (removedTotal + pendingInBatch)
                  << "] coverage=" << newCov.coverage
                  << " baseline=" << baseline << "\n";

        if (newCov.coverage + 1e-9 < baseline) {
          tlog_info << "[WARN] Drop detected → rollback last " << pendingInBatch
                    << " removals\n";
          steps = std::move(snapSteps);
          freq = std::move(snapFreq);
          i = std::min(snapI + 1, steps.size());
          hasSnapshot = false;
          pendingInBatch = 0;
        } else {
          removedTotal += pendingInBatch;
          hasSnapshot = false;
          pendingInBatch = 0;
        }
      }
    } else {
      tlog_info << "  [Slice-Batch-Trace] Keeping  step " << i
                << " (uniq=" << uniq << ", dup=" << dup << ")\n";
      ++i;
    }
  }

  std::vector<recorder::Action> result;
  result.reserve(steps.size());
  for (auto& s : steps)
    result.push_back(s.action);

  auto finalCov = settings.objFunc(result);
  tlog_info << "[Slice-Batch-Trace] Final length=" << result.size()
            << " coverage=" << finalCov.coverage << " baseline=" << baseline
            << "\n";

  return result;
}

}  // namespace dslicer
}  // namespace cider
