#include "scenario.h"

#include <iostream>
#include <sstream>

namespace cider {
namespace qleaning {

std::string actionToShortString(const QAction& action) {
  return std::visit(
      [](auto&& act) -> std::string { return std::to_string(act.index); },
      action);
}

std::string actionsToString(const QActionList& actions) {
  std::ostringstream oss;
  for (size_t i = 0; i < actions.size(); ++i) {
    oss << actionToShortString(actions[i]) << ";";
  }
  if (actions.empty()) {
    oss << "Empty";
  }
  return oss.str();
}

Scenario::Scenario(const QActionList& initial, const ObjectiveFunction& objFunc)
    : m_size(initial.size()), m_objFunc(objFunc) {
  for (const auto& action : initial) {
    m_availableActions.emplace(action);
  }

  const auto objValue = m_objFunc(initial);
  if (objValue > std::numeric_limits<double>::epsilon()) {
    std::cout << "Initial: " << objValue << std::endl;
    m_initialObjVal = objValue;
  } else {
    throw std::logic_error{"Bad initial script."};
  }
}

void Scenario::add(const QAction& action) {
  m_actions.push_back(action);

  const auto avIt = m_availableActions.find(action);
  if (avIt != m_availableActions.cend()) {
    m_availableActions.erase(avIt);
  }
}

void Scenario::rollback() {
  const auto& action = m_actions.back();
  m_availableActions.emplace(action);

  m_actions.pop_back();
}

QActionList Scenario::getCurrentState() const {
  return m_actions;
}

bool Scenario::isOver() const {
  return m_lastObjVal > m_initialObjVal ||
         (abs(m_lastObjVal - m_initialObjVal) <
          std::numeric_limits<double>::epsilon()) ||
         m_size < m_actions.size();  // try >=
}

std::optional<QValue> Scenario::getReward() const {
  const auto objValue = m_objFunc(m_actions);
  if (objValue > std::numeric_limits<double>::epsilon()) {
    std::cout << "Candidate: " << objValue << std::endl;
    if (objValue > m_lastObjVal) {
      m_lastObjVal = objValue;
      return 1;  // cov increase
    } else {
      m_lastObjVal = objValue;
      return 0.2;  // ok script, but lower coverage
    }
  } else {
    return std::nullopt;  // bad script
  }
}

std::string Scenario::toString() const {
  return actionsToString(m_actions);
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
