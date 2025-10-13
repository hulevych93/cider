// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "qtable-agent.h"

#include "agent-model/scenario.h"

#include "serialization/deserializer.h"
#include "serialization/serializer.h"

#include <iomanip>
#include <random>

#include "math/softmax.h"

#include <tlog.h>

namespace cider {
namespace agent_model {

namespace {

QTableStats collectStats(const QTable& table) {
  QTableStats s{};
  s.numStates = table.size();
  if (s.numStates == 0)
    return s;

  s.minActionsInState = std::numeric_limits<std::size_t>::max();

  for (const auto& tIt : table) {
    const auto& qvals = tIt.second;

    const std::size_t n = qvals.size();
    s.totalActions += n;
    s.maxActionsInState = std::max(s.maxActionsInState, n);
    s.minActionsInState = std::min(s.minActionsInState, n);
    if (n == 0)
      ++s.zeroActionStates;
  }

  s.avgActionsPerState =
      static_cast<double>(s.totalActions) / static_cast<double>(s.numStates);
  if (s.minActionsInState == std::numeric_limits<std::size_t>::max())
    s.minActionsInState = 0;

  return s;
}

static std::string tsISO(const std::chrono::system_clock::time_point& tp) {
  const std::time_t tt = std::chrono::system_clock::to_time_t(tp);
  std::tm tm{};
  localtime_r(&tt, &tm);

  char buf[32];
  std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);
  return buf;
}

}  // namespace

QTableAgent::QTableAgent(const std::string& path)
    : _gen(Seed::instance().get()), m_loaded(load(path)), m_path(path) {
  tlog_info << "Load agent: " << path << ", status: " << m_loaded << std::endl;
}

bool QTableAgent::load(const std::string& filePath) {
  try {
    serialization::Deserializer deserializer(filePath);
    deserializer >> m_qtable;

    for (const auto& entry : m_qtable) {
      m_maxStateDepth = std::max(m_maxStateDepth, entry.first.size());
    }
    tlog_info << "Max recorder::Actions size: " << m_maxStateDepth << std::endl;

  } catch (const std::exception& e) {
    tlog_info << e.what() << std::endl;
    return false;
  }
  return true;
}

bool QTableAgent::save() const {
  try {
    serialization::Serializer serializer;
    serializer << m_qtable;
    serializer.save(m_path);
  } catch (...) {
    return false;
  }
  return true;
}

QValues getAvailableQValues(const recorder::Actions& available,
                            const QValues& values) {
  QValues availableValues;
  for (const auto& action : available) {
    const auto qValueIter = values.find(action);
    if (qValueIter != values.cend()) {
      availableValues.emplace(action, qValueIter->second);
    } else {
      availableValues.emplace(action, 0.0001);
    }
  }
  return availableValues;
}

recorder::Actions QTableAgent::getBestFromAvailable(
    const recorder::Actions& available,
    const QValues& values) {
  QValues availableValues;
  for (const auto& action : available) {
    const auto qValueIter = values.find(action);
    if (qValueIter != values.cend()) {
      availableValues.emplace(action, qValueIter->second);
    }
  }

  recorder::Actions bestValues;
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

std::optional<recorder::Action> QTableAgent::findBestOrRandomAvailableAction(
    const Scenario& scenario) const {
  const auto state = scenario.getCurrentState();
  const auto qValuesIter = m_qtable.find(state);
  const auto& availableActions = scenario.getAvailableActions();
  if (availableActions.empty()) {
    return std::nullopt;
  }
  if (qValuesIter != m_qtable.cend()) {
    const auto& qValues =
        getBestFromAvailable(availableActions, qValuesIter->second);
    if (!qValues.empty()) {
      tlog_info << "E-Greedy action" << std::endl;
      std::uniform_int_distribution<size_t> indexDist(0, qValues.size() - 1);
      return qValues[indexDist(_gen)];
    }

    return std::nullopt;
  } else {
    tlog_info << "fallback to random: 1" << std::endl;
    std::uniform_int_distribution<size_t> indexDist(
        0, availableActions.size() - 1);
    return availableActions[indexDist(_gen)];
  }
}

std::optional<recorder::Action> QTableAgent::chooseEGreedyAction(
    const Scenario& scenario,
    const double exploration) const {
  std::optional<recorder::Action> action;
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  if (dist(_gen) < exploration) {
    action = scenario.getRandomAction();
  }
  if (!action.has_value()) {
    action = findBestOrRandomAvailableAction(scenario);
  }
  return action;
}

std::optional<recorder::Action> QTableAgent::chooseGreedyAction(
    const Scenario& scenario) const {
  return findBestOrRandomAvailableAction(scenario);
}

std::optional<recorder::Action> QTableAgent::chooseBolzmanAction(
    const Scenario& scenario,
    const double temperature) const {
  std::optional<recorder::Action> action;
  const auto qValuesIt = m_qtable.find(scenario.getCurrentState());
  if (qValuesIt == m_qtable.cend()) {
    tlog_info << "fallback to random: 1" << std::endl;
    action = scenario.getRandomAction();
  } else {
    const auto& availableActions = scenario.getAvailableActions();
    if (availableActions.empty()) {
      return std::nullopt;
    }

    const auto& qValues =
        getAvailableQValues(availableActions, qValuesIt->second);

    if (qValues.empty()) {
      tlog_info << "fallback to random: 2" << std::endl;
      return scenario.getRandomAction();
    }

    std::vector<std::pair<const recorder::Action*, float>> pool;
    pool.reserve(qValues.size());
    for (const auto& kv : qValues) {
      pool.emplace_back(std::addressof(kv.first), kv.second);
    }

    if (pool.empty()) {
      tlog_info << "fallback to random: 3" << std::endl;
      return scenario.getRandomAction();
    }

    auto winner = math_stat::softmax_choice(
        pool, [](const auto& p) { return static_cast<double>(p.second); },
        temperature, _gen);

    tlog_info << "Boltzmann action" << std::endl;
    return *winner.first;
  }

  return action;
}

void QTableAgent::print(std::ostream& ss) const {
  ss << "Q-table: " << m_qtable.size() << std::endl;
  for (const auto& entry : m_qtable) {
    ss << actionsToGenericRepro(entry.first) << std::endl;
    for (const auto action : entry.second) {
      ss << actionToGenericRepro(action.first) << "\t" << action.second
         << std::endl;
    }
    ss << std::endl;
  }
}

void QTableAgent::printMetrics(std::ostream& os, const Episode& episode) const {
  const QTableStats s = collectStats(m_qtable);

  const std::string tStart = tsISO(episode.start);
  const std::string tEnd = tsISO(episode.end);
  const auto learningSec = std::chrono::duration_cast<std::chrono::seconds>(
                               episode.end - episode.start)
                               .count();

  os << episode.episode << ";" << tStart << ";" << tEnd << ";" << learningSec
     << ";" << s.numStates << ";" << s.totalActions << ";" << std::fixed
     << std::setprecision(6) << s.avgActionsPerState << ";"
     << s.maxActionsInState << ";" << s.minActionsInState << ";"
     << s.zeroActionStates << '\n';
}

}  // namespace agent_model
}  // namespace cider
