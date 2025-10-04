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
    *settings.openers = std::vector<double>(actionSpace.size(), 0.0);
  }

  struct Candidate final {
    int index = -1;
    bool hasUniqueBlock = false;
    double branchGain = 0.0;
  };
  std::vector<Candidate> lastRun(actionSpace.size());

  auto getObjective = [&](const Candidate& c) -> double {
    return c.branchGain + settings.lambda * (*settings.openers)[c.index];
  };

  size_t previousIndex = 0;
  double currentCoverage = 0.0;

  tlog_info << "[Greedy-BB-SM] Target covered branches: " << fullCoverage
            << "\n";

  size_t step = 0;

  const auto hasUniqueBlock = [&](const int i) {
    auto trial = selected;
    trial.push_back(actionSpace[i]);

    const auto fine = settings.fineObjFunc(trial);
    return fine.hasUnique;
  };

  int lastPoolSize = -1;

  while (true) {
    std::vector<Candidate> candidates;
    candidates.reserve(actionSpace.size());

    for (size_t i = 0; i < actionSpace.size(); ++i) {
      if (used[i])
        continue;

      Candidate candidate;
      candidate.index = i;
      candidate.hasUniqueBlock = hasUniqueBlock(i);
      candidates.push_back(candidate);
    }

    tlog_info << "[Greedy-BB-SM] Step " << step + 1
              << ": total candidates=" << candidates.size() << "\n";

    if (candidates.empty()) {
      tlog_info << "[Greedy-BB-SM] No further actions, stopping.\n";
      break;
    }

    std::vector<Candidate*> pool;

    if (lastPoolSize != -1) {
      pool.reserve(lastPoolSize);
    }

    for (auto& c : candidates) {
      if (c.hasUniqueBlock || (*settings.openers)[c.index] > 0.0) {
        pool.push_back(std::addressof(c));

        auto trial = selected;
        trial.push_back(actionSpace[c.index]);

        c.branchGain = settings.objFunc(trial).coverage - currentCoverage;
      }

      if (lastRun[c.index].index != -1 && !lastRun[c.index].hasUniqueBlock &&
          c.hasUniqueBlock) {
        if ((*settings.openers)[previousIndex] < c.branchGain) {
          (*settings.openers)[previousIndex] = c.branchGain;
          tlog_info << "[Greedy-BB-SM]  Index - " << previousIndex
                    << ", future bonus: " << c.branchGain << std::endl;
        }
      }

      lastRun[c.index] = c;
    }

    if (lastPoolSize < pool.size()) {
      lastPoolSize = pool.size();
    }

    if (pool.empty()) {
      tlog_info << "[Greedy-BB-SM] No candidates. ";
      break;
    } else {
      tlog_info << "[Greedy-BB-SM] Unique-block candidates: " << pool.size()
                << "/" << candidates.size() << "\n";
    }

    std::sort(pool.begin(), pool.end(),
              [&](const Candidate* a, const Candidate* b) {
                return getObjective(*a) > getObjective(*b);
              });

    size_t k = std::min(settings.top_k, pool.size());
    pool.erase(pool.cbegin() + k, pool.cend());

    for (auto& c : pool) {
      tlog_info << "   [Candidate] #" << c->index
                << " unique=" << c->hasUniqueBlock
                << " branchGain=" << c->branchGain
                << " objective=" << getObjective(*c) << "\n";
    }

    // --- softmax ---
    std::vector<double> probs(pool.size());
    double sum = 0.0;
    for (size_t k = 0; k < pool.size(); ++k) {
      double val = std::exp(getObjective(*pool[k]) /
                            std::max(1e-9, settings.temperature));
      probs[k] = val;
      sum += val;
    }

    for (auto& p : probs)
      p /= sum;

    std::discrete_distribution<size_t> dist(probs.begin(), probs.end());
    size_t chosen = dist(gen);
    const Candidate* winner = pool[chosen];

    used[winner->index] = true;

    previousIndex = winner->index;
    selected.push_back(actionSpace[winner->index]);

    currentCoverage = settings.objFunc(selected).coverage;

    tlog_info << "[Selection] Chosen action #" << winner->index
              << " branchGain=" << winner->branchGain
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
