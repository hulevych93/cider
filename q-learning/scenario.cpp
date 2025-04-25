#include "scenario.h"

#include <iostream>
#include <sstream>

namespace cider {
namespace qleaning {

std::string actionToShortString(const QAction& action) {
    return "A" + std::visit([](auto&& act) -> std::string { return std::to_string(act.index); }, action);
}

std::string actionToFullString(const QAction& action) {
    return std::visit([](auto&& act) -> std::string {
        using T = std::decay_t<decltype(act)>;

        if constexpr (std::is_same_v<T, cider::recorder::Function>) {
            std::ostringstream oss;
            oss << act.name << "(";
            for (size_t i = 0; i < act.params.size(); ++i) {
                oss << "x" + std::to_string(i);
                if (i + 1 < act.params.size()) oss << ", ";
            }
            oss << ")";
            return oss.str();
        }
        else if constexpr (std::is_same_v<T, cider::recorder::ClassMethod>) {
            std::ostringstream oss;
            oss << "obj@" << act.objectAddress << "->" << act.method.name << "(";
            for (size_t i = 0; i < act.method.params.size(); ++i) {
                oss << "x" + std::to_string(i);
                if (i + 1 < act.method.params.size()) oss << ", ";
            }
            oss << ")";
            return oss.str();
        }
        else if constexpr (std::is_same_v<T, cider::recorder::ClassBinaryOp>) {
            std::string opStr = (act.opName == cider::recorder::BinaryOpType::Assignment) ? "=" : "?";
            std::ostringstream oss;
            oss << "obj@" << act.objectAddress << " " << opStr << " " << "x0";
            return oss.str();
        }
        else if constexpr (std::is_same_v<T, cider::recorder::ClassUnaryOp>) {
            std::string opStr = (act.opName == cider::recorder::UnaryOpType::Minus) ? "-" : "?";
            std::ostringstream oss;
            oss << opStr << "obj@" << act.objectAddress;
            return oss.str();
        }
        else if constexpr (std::is_same_v<T, cider::recorder::ClassDestructor>) {
            return "destroy(obj@" + std::to_string(reinterpret_cast<uintptr_t>(act.objectAddress)) + ")";
        }
        else {
            return "UnknownAction";
        }
    }, action);
}

Scenario::Scenario(const QActionList& initial,
                   const MeassureCallback& meassurer)
    : m_size(initial.size()), m_meassurer(meassurer) {
  for (const auto& action : initial) {
    m_availableActions.emplace(action);
  }

  if (const auto& covOpt = m_meassurer(initial)) {
    m_initialCov = covOpt.value();
    cider::coverage::printTableEntry(std::cout, 0, m_initialCov.report);
  } else {
    throw std::logic_error{"Bad initial script."};
  }
}

void Scenario::add(const QAction& action) {
  m_actions.push_back(action);
}

void Scenario::rollback() {
    m_actions.pop_back();
}

bool Scenario::isOver() const {
  return m_lastCov > m_initialCov || m_lastCov == m_initialCov || m_size < m_actions.size();  // try >=
}

std::optional<QValue> Scenario::getReward() const {
  if (const auto& covOpt = m_meassurer(m_actions)) {
    const auto& currentCov = covOpt.value();
    if(currentCov.report.isNull()) {
        return std::nullopt; // bad script
    } else {
        cider::coverage::printTableEntry(std::cout, 0, currentCov.report);
        if (currentCov > m_lastCov) {
            m_lastCov = currentCov;
            return 1;  // cov increase
        } else {
            m_lastCov = currentCov;
            return 0.2;  // ok script, but lower coverage
        }
    }
  } else {
    return std::nullopt;  // bad script
  }
}

std::string Scenario::toString() const {
    return actionsToString(m_actions, actionToShortString);
}

QActionList Scenario::getAvailableActions() const {
  QActionList actions;
  actions.reserve(m_availableActions.size());
  for (const auto& action : m_availableActions) {
    actions.emplace_back(action);
  }
  return actions;
}

QAction Scenario::getRandomAction() const {
  const auto actions = getAvailableActions();
  return actions[rand() % actions.size()];
}

}  // namespace qleaning
}  // namespace cider
