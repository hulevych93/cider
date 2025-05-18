#include "scenario.h"

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

std::string actionToGenericRepro(const QAction& action) {
  std::stringstream os;
  std::visit(
      [&os](auto&& value) {
        using T = std::decay_t<decltype(value)>;

        auto printParams = [&os](const auto& args) {
          os << "(";
          for (size_t i = 0; i < args.size(); ++i) {
            os << "_";  // assuming `typeName` gives a string like "int"
            if (i + 1 != args.size())
              os << ", ";
          }
          os << ")";
        };

        if constexpr (std::is_same_v<T, cider::recorder::Function>) {
          os << value.name;
          printParams(value.params);  // assuming .arguments is a list of types
        } else if constexpr (std::is_same_v<T, cider::recorder::ClassMethod>) {
          os << "obj@->" << value.method.name;
          printParams(value.method.params);  // nested method inside ClassMethod
        } else if constexpr (std::is_same_v<T,
                                            cider::recorder::ClassBinaryOp>) {
          std::string opStr =
              (value.opName == cider::recorder::BinaryOpType::Assignment) ? "="
                                                                          : "?";
          os << "obj@ " << opStr;
        } else if constexpr (std::is_same_v<T, cider::recorder::ClassUnaryOp>) {
          std::string opStr =
              (value.opName == cider::recorder::UnaryOpType::Minus) ? "-" : "?";
          os << opStr << "obj@";
        } else if constexpr (std::is_same_v<T,
                                            cider::recorder::ClassDestructor>) {
          os << "destroy(obj@)";
          // likely no params for destructor
        } else {
          static_assert(!sizeof(T), "Unsupported Action type");
        }
      },
      action);
  return os.str();
}

std::string actionsToGenericRepro(const QActionList& actions) {
  if (actions.empty()) {
    return "Empty";
  }
  std::string result;
  for (const auto& action : actions) {
    result += actionToGenericRepro(action) + ';';
  }
  result.pop_back();
  return result;
}

}  // namespace qleaning
}  // namespace cider
