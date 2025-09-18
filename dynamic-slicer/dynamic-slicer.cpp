// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "dynamic-slicer.h"

#include <algorithm>
#include <iostream>
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

  std::cout << "[Slice] Original length: " << actionSpace.size()
            << ", branches covered %: " << targetCovered << "\n";

  for (size_t i = 0; i < sliced.size();) {
    std::vector<recorder::Action> trial = sliced;
    trial.erase(trial.begin() + i);

    auto cov = settings.objFunc(trial).coverage;

    if (cov >= targetCovered) {
      std::cout << "  [Slice] Removing action #" << i << " → OK\n"
                << ", total covered = " << cov << "/" << targetCovered << "\n";
      sliced = std::move(trial);
    } else {
      ++i;
    }
  }

  const auto targetCoveredRecheck = settings.objFunc(actionSpace).coverage;
  auto finalCov = settings.objFunc(sliced).coverage;
  if (finalCov < targetCoveredRecheck) {
    std::cout << "[Slice] Recheck FAILED: final=" << finalCov
              << " < target=" << targetCoveredRecheck
              << ". Rollback to original.\n";
  }

  std::cout << "[Slice] Final length: " << sliced.size() << "\n";
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

  std::cout << "[Slice-Batch] Original length=" << currentSpace.size()
            << " baseline=" << baseline << "\n";

  size_t i = 0;
  while (i < currentSpace.size()) {
    size_t batchEnd = std::min(i + settings.batchSize, currentSpace.size());
    std::vector<recorder::Action> removedBatch(currentSpace.begin() + i,
                                               currentSpace.begin() + batchEnd);

    currentSpace.erase(currentSpace.begin() + i,
                       currentSpace.begin() + batchEnd);

    auto newCov = settings.objFunc(currentSpace);
    std::cout << "  [Check@" << (i + removedBatch.size())
              << "] coverage=" << newCov.coverage << " baseline=" << baseline
              << "\n";

    if (newCov.coverage + 1e-9 < baseline) {
      std::cout << "[WARN] Drop detected → rollback batch ("
                << removedBatch.size() << ")\n";

      currentSpace.insert(currentSpace.begin() + i, removedBatch.begin(),
                          removedBatch.end());
      i = batchEnd;
    }
  }

  auto finalCov = settings.objFunc(currentSpace);
  std::cout << "[Slice-Batch] Final length=" << currentSpace.size()
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

    std::cout << "[Multipass-Batch-Percent] length=" << n << " step=" << step
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

std::vector<recorder::Action> run_d_slicing_fast_checked_tracks(
    const BatchTracksDSlicingSettings& settings,
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

  FineObjectiveValue fineRes = settings.fineObjFunc(currentSpace);
  if (fineRes.fineCoveredTracks.empty())
    return currentSpace;

  size_t trackCount = 0;
  for (auto& cov : fineRes.fineCoveredTracks) {
    if (!cov.empty()) {
      trackCount = cov.size();
      break;
    }
  }
  if (trackCount == 0) {
    std::cout << "[Slice-Batch-Trace] No coverage tracks found\n";
    return currentSpace;
  }

  struct Step final {
    recorder::Action action;
    std::vector<std::uint8_t> coverage;
  };

  std::vector<Step> steps;
  steps.reserve(currentSpace.size());

  for (size_t i = 0; i < currentSpace.size(); ++i) {
    std::vector<std::uint8_t> cov;
    if (fineRes.fineCoveredTracks[i].empty()) {
      cov.assign(trackCount, 0);
    } else {
      cov = fineRes.fineCoveredTracks[i];
    }
    steps.push_back({currentSpace[i], std::move(cov)});
  }

  std::cout << "[Slice-Batch-Trace] Original length=" << steps.size()
            << " tracks=" << trackCount << " baseline=" << baseline << "\n";

  std::vector<int> freq(trackCount, 0);
  for (auto& s : steps)
    for (size_t j = 0; j < trackCount; ++j)
      if (s.coverage[j])
        ++freq[j];

  size_t removedCount = 0;
  std::vector<Step> lastRemoved;

  for (size_t i = 0; i < steps.size();) {
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

      std::cout << "  [Slice-Batch-Trace] Removing step " << i
                << " (uniq=" << uniq << ", dup=" << dup << ")\n";

      lastRemoved.push_back(steps[i]);
      steps.erase(steps.begin() + i);
      ++removedCount;

      if (removedCount % 5 == 0) {
        auto scenario = [&]() {
          std::vector<recorder::Action> out;
          for (auto& s : steps)
            out.push_back(s.action);
          return out;
        }();

        auto newCov = settings.objFunc(scenario);
        std::cout << "  [Check@" << removedCount
                  << "] coverage=" << newCov.coverage
                  << " baseline=" << baseline << "\n";

        if (newCov.coverage + 1e-9 < baseline) {
          std::cout << "[WARN] Drop detected → rollback last "
                    << lastRemoved.size() << " removals\n";

          for (auto& s : lastRemoved) {
            steps.insert(steps.begin() + i, s);
            for (size_t j = 0; j < trackCount; ++j)
              if (s.coverage[j])
                ++freq[j];
            ++i;
          }
        }
        lastRemoved.clear();
      }
    } else {
      std::cout << "  [Slice-Batch-Trace] Keeping  step " << i
                << " (uniq=" << uniq << ", dup=" << dup << ")\n";
      ++i;
    }
  }

  std::vector<recorder::Action> result;
  result.reserve(steps.size());
  for (auto& s : steps)
    result.push_back(s.action);

  auto finalCov = settings.objFunc(result);
  std::cout << "[Slice-Batch-Trace] Final length=" << result.size()
            << " coverage=" << finalCov.coverage << " baseline=" << baseline
            << "\n";

  return result;
}

}  // namespace dslicer
}  // namespace cider
