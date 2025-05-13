// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "qtable-agent.h"

#include "scenario.h"

#include "serialization/deserializer.h"
#include "serialization/serializer.h"

#include <iostream>
#include <random>

namespace cider {
namespace qleaning {

QTableAgent::QTableAgent(const std::string& path)
    : _gen(rd()), m_loaded(load(path)) {
  std::cout << "Load agent: " << path << ", status: " << m_loaded << std::endl;
}

bool QTableAgent::load(const std::string& filePath) {
  try {
    serialization::Deserializer deserializer(filePath);
    deserializer >> m_qtable;
  } catch (...) {
    return false;
  }
  return true;
}

bool QTableAgent::save(const std::string& filePath) const {
  try {
    serialization::Serializer serializer;
    serializer << m_qtable;
    serializer.save(filePath);
  } catch (...) {
    return false;
  }
  return true;
}

QActionList QTableAgent::getBestFromAvailable(const QActionList& available,
                                              const QValues& values) {
  QValues availableValues;
  for (const auto& action : available) {
    const auto qValueIter = values.find(action);
    if (qValueIter != values.cend()) {
      availableValues.emplace(action, qValueIter->second);
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

std::optional<QAction> QTableAgent::findBestOrRandomAvailableAction(
    const Scenario& scenario) const {
  const auto state = scenario.toString();
  const auto qValuesIter = m_qtable.find(state);
  const auto& availableActions = scenario.getAvailableActions();
  if (availableActions.empty()) {
    return std::nullopt;
  }
  if (qValuesIter != m_qtable.cend()) {
    const auto& qValues =
        getBestFromAvailable(availableActions, qValuesIter->second);
    std::uniform_int_distribution<size_t> indexDist(0, qValues.size() - 1);
    return qValues[indexDist(_gen)];
  } else {
    std::uniform_int_distribution<size_t> indexDist(
        0, availableActions.size() - 1);
    return availableActions[indexDist(_gen)];
  }
}

std::optional<QAction> QTableAgent::chooseEGreedyAction(
    const Scenario& scenario,
    const double exploration) const {
  std::optional<QAction> action;
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  if (dist(_gen) < exploration) {
    action = scenario.getRandomAction();
  }
  if (!action.has_value()) {
    action = findBestOrRandomAvailableAction(scenario);
  }
  return action;
}

std::optional<QAction> QTableAgent::chooseGreedyAction(
    const Scenario& scenario) const {
  return findBestOrRandomAvailableAction(scenario);
}

std::optional<QAction> QTableAgent::chooseRandAction(
    const Scenario& scenario) const {
  return scenario.getRandomAction();
}

std::optional<QAction> QTableAgent::chooseBolzmanAction(
    const Scenario& scenario,
    const double temperature) const {
  std::optional<QAction> action;
  const auto qValuesIt = m_qtable.find(scenario.toString());
  if (qValuesIt == m_qtable.cend()) {
    action = scenario.getRandomAction();
  } else {
    const auto& qValues = qValuesIt->second;

    if (qValues.empty()) {
      return scenario.getRandomAction();
    }

    std::vector<float> probabilities(qValues.size());
    float sum = 0.0f;
    auto qValIt = qValues.begin();
    for (size_t i = 0; i < qValues.size(); ++i, ++qValIt) {
      const auto& value = qValIt->second;
      probabilities[i] = std::exp(value / temperature);
      sum += probabilities[i];
    }

    if (sum == 0.0f || std::isinf(sum)) {
      return scenario.getRandomAction();
    }

    for (float& p : probabilities)
      p /= sum;

    std::discrete_distribution<int> dist(probabilities.begin(),
                                         probabilities.end());
    const auto index = dist(_gen);

    qValIt = qValues.begin();
    size_t i = 0;
    for (; i < index; ++i, ++qValIt)
      ;

    action = qValIt->first;
  }

  return action;
}

double QTableAgent::updateQValues(const std::string& state,
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

  std::cout << "lr: " << learningRate << ", r: " << reward
            << ", mV: " << maxQValue << ", qv: " << qValue << " -> ";

  qValue += learningRate * (reward + discount * maxQValue - qValue);

  float target = reward + discount * maxQValue;
  float loss = 0.5f * (qValue - target) * (qValue - target);

  std::cout << qValue << ", target:" << target << ", loss: " << loss
            << std::endl;

  return loss;
}

void QTableAgent::print(std::ostream& ss) const {
  ss << "Q-table: " << m_qtable.size() << std::endl;
  for (const auto& entry : m_qtable) {
    ss << entry.first << std::endl;
    for (const auto action : entry.second) {
      ss << actionToGenericRepro(action.first) << "\t" << action.second
         << std::endl;
    }
    ss << std::endl;
  }
}

}  // namespace qleaning
}  // namespace cider
