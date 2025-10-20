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

using QActionsPool = std::vector<std::pair<const recorder::Action*, double>>;

bool hasValidQInfo(
    const std::vector<std::pair<const recorder::Action*, double>>& pool) {
  for (auto& p : pool)
    if (std::abs(p.second) > 1e-15)
      return true;
  return false;
}

const recorder::Action* chooseGreedy(const QActionsPool& pool,
                                     std::mt19937& gen) {
  auto maxIt = std::max_element(pool.begin(), pool.end(), [](auto& a, auto& b) {
    return a.second < b.second;
  });

  if (maxIt == pool.end() || std::abs(maxIt->second) < 1e-15)
    return nullptr;

  std::vector<const recorder::Action*> best;
  for (auto& p : pool)
    if (std::abs(p.second - maxIt->second) < 1e-9)
      best.push_back(p.first);

  std::uniform_int_distribution<size_t> dist(0, best.size() - 1);
  return best[dist(gen)];
}

const recorder::Action* chooseSoftmax(const QActionsPool& pool,
                                      double temperature,
                                      std::mt19937& gen) {
  if (!hasValidQInfo(pool))
    return nullptr;

  auto winner = math_stat::softmax_choice(
      pool, [](const auto& p) { return static_cast<double>(p.second); },
      temperature, gen);

  return winner.first;
}

QActionsPool blendQOverSuffixes(SuffixLogger logger,
                                const QTable& table,
                                const recorder::Actions& currentState,
                                const recorder::Actions& availableActions,
                                double beta = 0.7) {
  tlog_info << "[blendQ] state size=" << currentState.size()
            << ", table states=" << table.size() << std::endl;

  QValues acc, wsum;
  acc.reserve(availableActions.size());
  wsum.reserve(availableActions.size());

  for (size_t k = 0; k <= currentState.size(); ++k) {
    const auto key = takeSuffix(currentState, k);
    const auto it = table.find(key);
    if (it == table.end()) {
      tlog_info << "[blendQ]  k=" << k << "  ❌ no entry" << std::endl;
      continue;
    }

    if (logger) {
      logger(k);
    }

    const double N = static_cast<double>(it->second.size());
    const double w = std::pow(N + 1e-6, beta);

    tlog_info << "[blendQ]  k=" << k << "  ✅ found  actions=" << N
              << "  weight=" << w << std::endl;

    for (const auto& a : availableActions) {
      double q = 0.0001;
      auto qit = it->second.find(a);
      if (qit != it->second.end())
        q = qit->second;

      acc[a] += w * q;
      wsum[a] += w;
    }
  }

  QActionsPool pool;
  pool.reserve(availableActions.size());

  for (const auto& a : availableActions) {
    const double w = wsum[a];
    const double q = (w > 0.0 ? acc[a] / w : 0.0001);
    pool.emplace_back(&a, q);
  }

  if (pool.empty())
    tlog_info << "[blendQ] no blended Q-values (all zero or no suffix match)"
              << std::endl;

  return pool;
}

template <typename ChoosePolicy>
std::optional<recorder::Action> chooseActionBase(SuffixLogger logger,
                                                 std::mt19937& gen,
                                                 const QTable& table,
                                                 const Scenario& scenario,
                                                 ChoosePolicy chooser) {
  const auto& available = scenario.getAvailableActions();
  if (available.empty())
    return std::nullopt;

  const auto currentState = scenario.getCurrentState();
  const auto pool =
      blendQOverSuffixes(logger, table, currentState, available, 0.7);

  if (!hasValidQInfo(pool)) {
    tlog_info << "[Policy] no Q info → random fallback" << std::endl;
    return scenario.getRandomAction();
  }

  const recorder::Action* act = chooser(pool, gen);
  if (!act)
    return scenario.getRandomAction();
  return *act;
}

}  // namespace

QTableAgent::QTableAgent(const std::string& path, SuffixLogger suffixLogger)
    : _gen(Seed::instance().get()),
      m_loaded(load(path)),
      m_path(path),
      m_suffixLogger(suffixLogger) {
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

std::optional<recorder::Action> QTableAgent::chooseEGreedyAction(
    const Scenario& scenario,
    double exploration) const {
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  if (dist(_gen) < exploration) {
    tlog_info << "E-Greedy: random exploration" << std::endl;
    return scenario.getRandomAction();
  }

  tlog_info << "E-Greedy: greedy exploitation" << std::endl;
  return chooseActionBase(m_suffixLogger, _gen, m_qtable, scenario,
                          chooseGreedy);
}

std::optional<recorder::Action> QTableAgent::chooseGreedyAction(
    const Scenario& scenario) const {
  tlog_info << "Greedy(blended) policy" << std::endl;
  return chooseActionBase(m_suffixLogger, _gen, m_qtable, scenario,
                          chooseGreedy);
}

std::optional<recorder::Action> QTableAgent::chooseBoltzmannAction(
    const Scenario& scenario,
    double temperature) const {
  tlog_info << "Boltzmann(blended) policy" << std::endl;
  return chooseActionBase(m_suffixLogger, _gen, m_qtable, scenario,
                          [&](auto& pool, auto& gen) {
                            return chooseSoftmax(pool, temperature, gen);
                          });
}

std::optional<recorder::Action> QTableAgent::chooseBoltzmannWithOpenersAction(
    const Scenario& scenario,
    double temperature,
    double lambda,
    size_t top_k) const {
  const auto& available = scenario.getAvailableActions();
  if (available.empty())
    return std::nullopt;

  const auto currentState = scenario.getCurrentState();
  auto pool = blendQOverSuffixes(m_suffixLogger, m_qtable, currentState,
                                 available, 0.7);

  bool noQInfo = !hasValidQInfo(pool);

  if (noQInfo) {
    auto& openers = synthesis::Openers::get();
    auto candidates = scenario.getCandidates(openers);
    if (candidates.empty()) {
      tlog_info << "Boltzmann(Openers fallback): no candidates" << std::endl;
      return scenario.getRandomAction();
    }
    tlog_info << "Boltzmann(Openers fallback): pure openers mode" << std::endl;
    const auto winner = synthesis::chooseWithOpeners(
        _gen, candidates, openers, top_k, temperature, lambda);
    return winner.action;
  } else {
    auto& openers = synthesis::Openers::get();
    auto candidates = scenario.getCandidates(openers);

    if (!candidates.empty()) {
      auto getObjective = [&](const synthesis::Candidate& c) -> double {
        return openers.getObjective(lambda, c);
      };
      std::sort(candidates.begin(), candidates.end(),
                [&](const auto& a, const auto& b) {
                  return getObjective(a) > getObjective(b);
                });

      if (candidates.size() > top_k)
        candidates.resize(top_k);

      std::unordered_set<recorder::Action, recorder::FuzzyActionHash,
                         recorder::FuzzyEqualPred>
          topPicked;
      for (auto& c : candidates) {
        topPicked.insert(c.action);
      }

      auto minmaxIt = std::minmax_element(
          pool.begin(), pool.end(),
          [](auto& a, auto& b) { return a.second < b.second; });

      double range =
          std::max(1e-6, minmaxIt.first->second - minmaxIt.second->second);
      double lambda_auto = 0.1 * range;

      for (auto& p : pool)
        if (topPicked.count(*p.first))
          p.second += lambda_auto;

      tlog_info << "Boltzmann(Openers mixed): λ=" << lambda
                << ", top_k=" << top_k << ", adjusted " << topPicked.size()
                << " actions" << std::endl;
    }
  }

  tlog_info << "Boltzmann(backoff+prior) policy" << std::endl;
  auto act = chooseSoftmax(pool, temperature, _gen);
  if (!act)
    return scenario.getRandomAction();
  return *act;
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
  const auto collectStats = [&](const QTable& table) -> QTableStats {
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
  };

  const auto tsISO =
      [&](const std::chrono::system_clock::time_point& tp) -> std::string {
    const std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm tm{};
    localtime_r(&tt, &tm);

    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);
    return buf;
  };

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
