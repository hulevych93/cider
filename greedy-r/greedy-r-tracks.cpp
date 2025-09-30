// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "greedy-r-tracks.h"

#include "synthesis/synthesis.h"

#include <tlog.h>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <numeric>
#include <string>
#include <unordered_set>
#include <vector>

namespace cider {
namespace greedy_r {

std::ostream& operator<<(std::ostream& os,
                         const GreedyRTracksSettings& settings) {
  os << "GREEDY_R_";
  os << "top_k[";
  os << settings.top_k;
  os << "]temparature[";
  os << settings.temperature;
  os << "]";
  return os;
}

using TestCase = std::vector<recorder::Action>;

// Greedy Minimizer with Basic Blocks and Softmax (Boltzmann) Selection +
// Detailed Logs
TestCase run_greedy_r_tracks(std::mt19937& gen,
                             const GreedyRTracksSettings& settings,
                             const std::vector<recorder::Action>& actionSpace) {
  auto fullCoverage = settings.baseline;

  if (fullCoverage <= std::numeric_limits<double>::epsilon()) {
    auto baseCov = settings.objFunc(actionSpace);
    fullCoverage = baseCov.coverage;
  }

  std::vector<recorder::Action> selected;
  std::vector<bool> used(actionSpace.size(), false);
  if (settings.openers && (*settings.openers).empty()) {
    *settings.openers = std::vector<bool>(actionSpace.size(), false);
  }

  struct Candidate final {
    size_t index = 0;
    bool hasUniqueBlock = false;
    double branchGain = 0.0;
  };
  std::vector<Candidate> lastRun(actionSpace.size());

  size_t previousIndex = 0;
  double currentCoverage = 0.0;

  tlog_info << "[Greedy-BB-SM] Target covered branches: " << fullCoverage
            << "\n";

  size_t step = 0;

  const auto hasUniqueBlock = [&](const int i) {
    auto trial = selected;
    trial.push_back(actionSpace[i]);

    const auto fine = settings.fineObjFunc(trial);
    if (fine.fineCoveredTracks.empty()) {
      return false;
    }
    const auto& lastStep = fine.fineCoveredTracks.back();

    return std::any_of(lastStep.begin(), lastStep.end(),
                       [](uint8_t v) { return v == 1; });
  };

  while (true) {
    std::vector<Candidate> candidates;
    for (size_t i = 0; i < actionSpace.size(); ++i) {
      if (used[i])
        continue;

      Candidate candidate;
      candidate.index = i;
      candidate.hasUniqueBlock = hasUniqueBlock(i);

      if (!lastRun[i].hasUniqueBlock && candidate.hasUniqueBlock) {
        if (!(*settings.openers)[previousIndex]) {
          (*settings.openers)[previousIndex] = true;
          tlog_info << "[Greedy-BB-SM] Opener found at index - "
                    << previousIndex << std::endl;
        }
      }

      candidates.push_back(candidate);
      lastRun[i] = candidate;
    }

    tlog_info << "[Greedy-BB-SM] Step " << step + 1
              << ": total candidates=" << candidates.size() << "\n";

    if (candidates.empty()) {
      tlog_info << "[Greedy-BB-SM] No further actions, stopping.\n";
      break;
    }

    std::vector<Candidate> pool;

    bool openerPick = false;

    for (auto& c : candidates) {
      if (c.hasUniqueBlock)
        pool.push_back(c);
    }

    if (pool.empty()) {
      for (auto& c : candidates) {
        if ((*settings.openers)[c.index]) {
          pool.push_back(c);
        }
      }
      openerPick = true;
      if (pool.empty()) {
        tlog_info << "[Greedy-BB-SM] No candidates. ";
        break;
      }
    } else {
      tlog_info << "[Greedy-BB-SM] Unique-block candidates: " << pool.size()
                << "/" << candidates.size() << "\n";
    }

    for (auto& c : pool) {
      auto trial = selected;
      trial.push_back(actionSpace[c.index]);

      c.branchGain = settings.objFunc(trial).coverage - currentCoverage;
    }

    std::sort(pool.begin(), pool.end(),
              [](const Candidate& a, const Candidate& b) {
                return a.branchGain > b.branchGain;
              });

    size_t k = std::min(settings.top_k, pool.size());
    pool.erase(pool.cbegin() + k, pool.cend());

    for (auto& c : pool) {
      tlog_info << "   [Candidate] #" << c.index
                << " unique=" << c.hasUniqueBlock
                << " branchGain=" << c.branchGain
                << " isOpener=" << (*settings.openers)[c.index] << "\n";
    }

    // --- softmax ---
    std::vector<double> probs(pool.size());
    double sum = 0.0;
    for (size_t k = 0; k < pool.size(); ++k) {
      double val = 0;
      if (openerPick) {
        val = std::exp(1.0 / std::max(1e-9, settings.temperature));
      } else {
        val =
            std::exp(pool[k].branchGain / std::max(1e-9, settings.temperature));
      }
      probs[k] = val;
      sum += val;
    }

    for (auto& p : probs)
      p /= sum;

    std::discrete_distribution<size_t> dist(probs.begin(), probs.end());
    size_t chosen = dist(gen);
    const Candidate& winner = pool[chosen];

    used[winner.index] = true;

    previousIndex = winner.index;
    selected.push_back(actionSpace[winner.index]);

    currentCoverage = settings.objFunc(selected).coverage;

    tlog_info << "[Selection] Chosen action #" << winner.index
              << " branchGain=" << winner.branchGain
              << " prob=" << probs[chosen]
              << " branch Coverage=" << currentCoverage << "% \n";
    ++step;

    if (currentCoverage >= fullCoverage) {
      tlog_info << "[Greedy-BB-SM] Target coverage reached at step " << step
                << ".\n";
      break;
    }
  }

  tlog_info << "[Greedy-BB-SM] Final script length: " << selected.size()
            << " finalCoverage=" << currentCoverage << "/" << fullCoverage
            << "\n";

  return selected;
}

}  // namespace greedy_r
}  // namespace cider
