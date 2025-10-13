// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "greedy-r-tracks.h"

#include "synthesis/synthesis.h"

#include "math/softmax.h"

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
                             synthesis::Openers& openers,
                             const GreedyRTracksSettings& settings,
                             const std::vector<recorder::Action>& actionSpace) {
  synthesis::TestScenario scenario(gen, actionSpace, settings.objFunc,
                                   settings.fineObjFunc, settings.baseline);
  openers.init();

  size_t step = 0;

  auto getObjective = [&](const synthesis::Candidate& c) -> double {
    return openers.getObjective(settings.lambda, c);
  };

  while (true) {
    auto candidates = scenario.getCandidates(openers);

    if (candidates.empty()) {
      tlog_info << "[Greedy-BB-SM] Step " << step + 1
                << ". No further candidates, stopping.\n";
      break;
    }

    const auto winner =
        synthesis::chooseWithOpeners(gen, candidates, openers, settings.top_k,
                                     settings.temperature, settings.lambda);

    openers.setWinner(winner.action);

    scenario.add(winner.action);

    for (auto& c : candidates) {
      tlog_info << "   [Candidate] #" << actionToGenericRepro(c.action)
                << " unique=" << c.hasUniqueBlock
                << " branchGain=" << c.branchGain
                << " objective=" << getObjective(c) << "\n";
    }

    tlog_info << "[Selection] Step " << step + 1 << ", Chosen action #"
              << actionToGenericRepro(winner.action)
              << " branchGain=" << winner.branchGain << std::endl;
    ++step;

    if(!scenario.isValid(true)) {
        scenario.rollback();
    } else {
        if (scenario.isOver()) {
            tlog_info << "[Greedy-BB-SM] Target coverage reached at step " << step
                      << ".\n";
            break;
        }
    }
  }

  tlog_info << "[Greedy-BB-SM] Final script length: " << scenario.getSize()
            << std::endl;

  return scenario.getResult();
}

}  // namespace greedy_r
}  // namespace cider
