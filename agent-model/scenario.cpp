#include "scenario.h"

#include "reward_func.h"

#include <tlog.h>

namespace cider {
namespace agent_model {

Scenario::Scenario(std::mt19937& gen,
                   int maxStateDepth,
                   const recorder::Actions& initial,
                   const synthesis::ObjectiveFunction& objFunc)
    : synthesis::TestScenario(gen, initial, objFunc),
      _maxStateDepth(maxStateDepth) {}

recorder::Actions Scenario::getCurrentState() const {
  auto count = _maxStateDepth;
  if (count > _actions.size()) {
    count = _actions.size();
  }

  return recorder::Actions{_actions.end() - count, _actions.end()};
}

LearningScenario::LearningScenario(const RewardShappingParams& params,
                                   RewardCounter& counter,
                                   std::mt19937& gen,
                                   int maxStateDepth,
                                   const recorder::Actions& initial,
                                   const synthesis::ObjectiveFunction& objFunc)
    : Scenario(gen, maxStateDepth, initial, objFunc),
      _rwCounter(counter),
      _params(params) {}

std::optional<double> LearningScenario::getReward() const {
  const auto objValue = _objFunc(_actions);

  return getShapedReward(_params, _rwCounter, objValue, _lastObjVal,
                         _initialObjVal, _initialSize, _actions.size(),
                         calculateRedundancy(true), !_availableActions.empty());
}

size_t LearningScenario::calculateRedundancy(bool local) const {
  if (_actions.size() < 2)
    return 0;

  const auto getRedundancy = [](const recorder::Actions& actions) {
    int redundancy = 0;

    for (size_t i = 1; i < actions.size(); ++i) {
      if (cider::recorder::semanticallyEqual(actions[i], actions[i - 1])) {
        redundancy++;
      }
    }
    return redundancy;
  };

  if (local) {
    return getRedundancy(getCurrentState());
  } else {
    return getRedundancy(_actions);
  }
}

double LearningScenario::getCoverage(bool retry) const {
  auto cov = _objFunc(_actions).coverage;
  if (cov < 0.0000001 && retry) {
    return _objFunc(_actions).coverage;
  }
  return cov;
}

std::string actionToGenericRepro(const recorder::Action& action) {
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

std::string actionsToGenericRepro(
    const std::vector<recorder::Action>& actions) {
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

}  // namespace agent_model
}  // namespace cider
