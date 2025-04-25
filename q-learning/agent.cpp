// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "agent.h"

#include "scenario.h"

#include <iostream>

namespace cider {
namespace qleaning {

QActionList QValuesAgent::getBestFromAvailable(const QActionList& available,
                                               const QValues& values) {
  QValues availableValues;
  for (const auto& action : available) {
    const auto qValueIter = values.find(action);
    if (qValueIter != values.cend()) {
      availableValues.emplace(qValueIter->first, qValueIter->second);
    }
  }

  QActionList bestValues;
  const auto elemIter = std::max_element(
      availableValues.cbegin(), availableValues.cend(),
      [](const QValues::value_type& left, const QValues::value_type& right) {
        return left.second < right.second;
      });
  for (const auto value : availableValues) {
    if (value.second == elemIter->second) {
      bestValues.emplace_back(value.first);
    }
  }

  if (bestValues.empty()) {
    return available;
  }

  return bestValues;
}

QAction QValuesAgent::findBestOrRandomAvailableAction(
    const Scenario& scenario) const {
  const auto state = scenario.toString();
  const auto qValuesIter = m_qtable.find(state);
  const auto& availableActions = scenario.getAvailableActions();
  if (qValuesIter != m_qtable.cend()) {
    const auto& qValues =
        getBestFromAvailable(availableActions, qValuesIter->second);
    return qValues[rand() % qValues.size()];
  } else {
    return availableActions[rand() % availableActions.size()];
  }
}

void QValuesAgent::printAlternatives(const Scenario& scenario) const {
  const auto state = scenario.toString();
  const auto qValuesIter = m_qtable.find(state);
  if (qValuesIter != m_qtable.cend()) {
    for (const auto action : qValuesIter->second) {
      std::cout << actionToShortString(action.first) << " - " << action.second
                << std::endl;
    }
  }
}

QAction QValuesAgent::chooseAction(const Scenario& scenario,
                                   const double exploration) {
  QAction action;
  if (rand() / static_cast<double>(RAND_MAX) < exploration) {
    action = scenario.getRandomAction();
  } else {
    action = findBestOrRandomAvailableAction(scenario);
  }
  return action;
}

QAction QValuesAgent::chooseAction(const Scenario& scenario) const {
  return findBestOrRandomAvailableAction(scenario);
}

void QValuesAgent::updateQValues(const std::string& state,
                                 const std::string& nextState,
                                 const QAction& action,
                                 const double reward,
                                 const double learningRate,
                                 const double discount) {
  auto& qValues = m_qtable[state];
  auto& qValue = qValues[action];

  double maxQValue = 0;
  const auto qNextValuesIter = m_qtable.find(nextState);
  if (qNextValuesIter != m_qtable.cend()) {
    const auto& qNextValues = qNextValuesIter->second;
    for (const auto& qNextValue : qNextValues) {
      maxQValue = std::max(maxQValue, qNextValue.second);
    }
  }

  qValue += learningRate * reward;

  if (maxQValue != 0) {
    qValue += learningRate * (discount * maxQValue - qValue);
  }
}

void QValuesAgent::print(std::ostream& ss) const {
  ss << "Q-table: " << m_qtable.size() << std::endl;
  for (const auto& entry : m_qtable) {
      ss << entry.first << std::endl;
    for (const auto action : entry.second) {
      ss << actionToShortString(action.first) << " - " << action.second << std::endl;
    }
    ss << std::endl;
  }
}

QAction RandomAgent::chooseAction(const Scenario& scenario) const {
  return scenario.getRandomAction();
}

}  // namespace qleaning
}  // namespace cider
