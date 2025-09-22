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

std::ostream& operator<<(std::ostream& os, const DSlicingSettings& s) {
  os << "DSL";
  return os;
}

std::ostream& operator<<(std::ostream& os, const BatchDSlicingSettings& s) {
  os << "DSL-F"
     << "_checkStep[" << s.batchSize << "]";
  return os;
}

std::ostream& operator<<(std::ostream& os,
                         const BatchMultiPassDSlicingSettings& s) {
  os << "DSL-FM"
     << "_initStepRatio[" << s.initialStepRatio << "]"
     << "_minGran[" << s.minimalGranularity << "]"
     << "_baseline[" << s.baseline << "]";
  return os;
}

using TestCase = std::vector<recorder::Action>;

TestCase run_d_slicing(const DSlicingSettings& settings,
                       const std::vector<recorder::Action>& actionSpace) {
  auto targetCovered = settings.baseline;

  if (targetCovered <= std::numeric_limits<double>::epsilon()) {
    auto baseCov = settings.objFunc(actionSpace);
    targetCovered = baseCov.coverage;
  }

  const auto currentCoverage = settings.objFunc(actionSpace);
  if (currentCoverage.coverage < targetCovered) {
    return actionSpace;
  }

  std::vector<recorder::Action> sliced = deepCopy(actionSpace);

  tlog_info << "[Slice] Original length: " << actionSpace.size()
            << ", branches covered %: " << targetCovered << "\n";

  for (size_t i = 0; i < sliced.size();) {
    std::vector<recorder::Action> trial = sliced;
    trial.erase(trial.begin() + i);

    auto cov = settings.objFunc(trial).coverage;

    if (cov >= targetCovered) {
      tlog_info << "  [Slice] Removing action #" << i << " → OK\n"
                << ", total covered = " << cov << "/" << targetCovered << "\n";
      sliced = std::move(trial);
    } else {
      ++i;
    }
  }

  const auto targetCoveredRecheck = settings.objFunc(actionSpace).coverage;
  auto finalCov = settings.objFunc(sliced).coverage;
  if (finalCov < targetCoveredRecheck) {
    tlog_info << "[Slice] Recheck FAILED: final=" << finalCov
              << " < target=" << targetCoveredRecheck
              << ". Rollback to original.\n";
  }

  tlog_info << "[Slice] Final length: " << sliced.size() << "\n";
  return sliced;
}

std::vector<recorder::Action> run_d_slicing_batch(
    const BatchDSlicingSettings& settings,
    const std::vector<recorder::Action>& actionSpace) {
  auto currentSpace = deepCopy(actionSpace);

  auto baseline = settings.baseline;

  if (baseline <= std::numeric_limits<double>::epsilon()) {
    auto baseCov = settings.objFunc(currentSpace);
    baseline = baseCov.coverage;
  }

  const auto currentCoverage = settings.objFunc(currentSpace);
  if (currentCoverage.coverage < baseline) {
    return currentSpace;
  }

  tlog_info << "[Slice-Batch] Original length=" << currentSpace.size()
            << " baseline=" << baseline << "\n";

  size_t i = 0;
  while (i < currentSpace.size()) {
    size_t batchEnd = std::min(i + settings.batchSize, currentSpace.size());
    std::vector<recorder::Action> removedBatch(currentSpace.begin() + i,
                                               currentSpace.begin() + batchEnd);

    currentSpace.erase(currentSpace.begin() + i,
                       currentSpace.begin() + batchEnd);

    auto newCov = settings.objFunc(currentSpace);
    tlog_info << "  [Check@" << (i + removedBatch.size())
              << "] coverage=" << newCov.coverage << " baseline=" << baseline
              << "\n";

    if (newCov.coverage + 1e-9 < baseline) {
      tlog_info << "[WARN] Drop detected → rollback batch ("
                << removedBatch.size() << ")\n";

      currentSpace.insert(currentSpace.begin() + i, removedBatch.begin(),
                          removedBatch.end());
      i = batchEnd;
    }
  }

  auto finalCov = settings.objFunc(currentSpace);
  tlog_info << "[Slice-Batch] Final length=" << currentSpace.size()
            << " coverage=" << finalCov.coverage << " baseline=" << baseline
            << "\n";

  return currentSpace;
}

std::vector<recorder::Action> run_d_slicing_batch_multipass(
    const BatchMultiPassDSlicingSettings& settings,
    const std::vector<recorder::Action>& actionSpace) {
  std::vector<recorder::Action> current = deepCopy(actionSpace);

  auto baseline = settings.baseline;

  if (baseline <= std::numeric_limits<double>::epsilon()) {
    auto baseCov = settings.objFunc(actionSpace);
    baseline = baseCov.coverage;
  }

  auto initialStepRatio = settings.initialStepRatio;
  size_t n = current.size();

  while (true) {
    n = current.size();
    if (n <= 3)
      break;

    size_t step = static_cast<size_t>(n * initialStepRatio);
    if (step < settings.minimalGranularity)
      step = settings.minimalGranularity;

    tlog_info << "[Multipass-Batch-Percent] length=" << n << " step=" << step
              << " (" << initialStepRatio * 100 << "%)\n";

    BatchDSlicingSettings fastSettings;
    fastSettings.objFunc = settings.objFunc;
    fastSettings.fineObjFunc = settings.fineObjFunc;
    fastSettings.batchSize = step;
    fastSettings.baseline = baseline;

    current = run_d_slicing_batch(fastSettings, current);
    current = deepCopy(current);

    if (step == settings.minimalGranularity) {
      break;
    }
    initialStepRatio /= 2.0;
  }

  return current;
}

}  // namespace dslicer
}  // namespace cider
