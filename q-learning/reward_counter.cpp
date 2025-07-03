#include "reward_counter.h"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace cider {
namespace qleaning {

void print(std::ostream& os, const RewardCounter& rwCounter) {
  os << "==================== REWARD COUNTER ====================\n";
  os << std::setw(20) << std::left << "Coverage Grows:" << rwCounter.covGrow
     << '\n';
  os << std::setw(20) << std::left << "Track Grows:" << rwCounter.trackGrow
     << '\n';
  os << std::setw(20) << std::left
     << "Coverage Near Best:" << rwCounter.covNearBest << '\n';
  os << std::setw(20) << std::left
     << "Two Same Actions:" << rwCounter.twoSameAct << '\n';
  os << std::setw(20) << std::left
     << "Three Same Actions:" << rwCounter.threeSameAct << '\n';
  os << std::setw(20) << std::left << "Same Coverage:" << rwCounter.sameCov
     << '\n';
  os << std::setw(20) << std::left << "Penalties:" << rwCounter.penalty << '\n';
  os << std::setw(20) << std::left << "Script Lower:" << rwCounter.scriptLower
     << '\n';
  os << std::setw(20) << std::left << "Script Same:" << rwCounter.scriptSame
     << '\n';
  os << std::setw(20) << std::left << "Script Bigger:" << rwCounter.scriptBigger
     << '\n';
  os << "========================================================\n";
}

}  // namespace qleaning
}  // namespace cider
