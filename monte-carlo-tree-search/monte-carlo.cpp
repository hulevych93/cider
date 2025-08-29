// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "monte-carlo.h"

#include "synthesis/synthesis.h"

#include <iostream>

namespace cider {
namespace mcts {

constexpr bool MctsDebugEnable = false;

std::ostream& operator<<(std::ostream& os, const MonteCarloSettings& settings) {
  os << "MCTS_";
  os << "maxIter[";
  os << settings.maxIter;
  os << "]_maxRollback[";
  os << settings.maxRollback;
  os << "]_ucbC[";
  os << settings.ucb_C;
  os << "]";
  return os;
}

using TestCase = std::vector<recorder::Action>;

struct MCTSNode final {
  std::vector<recorder::Action> path;
  MCTSNode* parent = nullptr;
  std::unordered_map<recorder::Action,
                     std::unique_ptr<MCTSNode>,
                     recorder::FuzzyActionHash,
                     recorder::FuzzyEqualPred>
      children;
  int visits = 0;
  double total_reward = 0.0;
  bool fullyExpanded = false;

  double best_reward = -1e9;
  TestCase best_path;

  MCTSNode(TestCase p, MCTSNode* parent = nullptr)
      : path(std::move(p)), parent(parent) {}

  MCTSNode* best_child_ucb(double c);
};

double evaluate_reward(ObjectiveFunction objFunc, const TestCase& tc) {
  auto objValue = objFunc(tc);
  return objValue.coverage;
}

bool isValid(ObjectiveFunction objFunc, const TestCase& base) {
  const auto objValue = objFunc(base);
  if (objValue.coverage > std::numeric_limits<double>::epsilon()) {
    return true;
  }
  return false;
}

double rollout(std::mt19937& gen,
               ObjectiveFunction objFunc,
               TestCase& candidate,
               size_t max_rollback,
               size_t maxDepth,
               const std::vector<recorder::Action>& actionSpace) {
  synthesis::TestScenario scenario(gen, actionSpace, objFunc);
  for (const auto& a : candidate) {
    scenario.add(a);
  }

  const auto actionChoosing = [&](const synthesis::TestScenario& testCase) {
    return testCase.getRandomAction();
  };

  synthesis::SynthesisSettingsBasic settings;
  settings.maxRollback = max_rollback;
  settings.stopType = synthesis::StopCondition::LimitActions;
  settings.limitActions = maxDepth;
  synthesis::details::synthesize(settings, actionChoosing, scenario);

  candidate = scenario.getResult();

  double reward = evaluate_reward(objFunc, candidate);
  std::cout << "[Rollout] len=" << candidate.size() << ", reward=" << reward
            << "\n";
  return reward;
}

void backpropagate(MCTSNode* node, double reward, const TestCase& testCase) {
  while (node != nullptr) {
    node->visits++;
    node->total_reward += reward;

    std::cout << "Backprop @node depth=" << node->path.size()
              << ", rollout len=" << testCase.size() << ", reward=" << reward
              << ", prev_best=" << node->best_reward << "\n";

    // update best reward + best path if needed
    if (reward > node->best_reward) {
      node->best_reward = reward;
      node->best_path = testCase;
    }

    node = node->parent;
  }
}

MCTSNode* MCTSNode::best_child_ucb(double c) {
  MCTSNode* best = nullptr;
  double best_score = -1e9;

  for (const auto& it : children) {
    const auto& child = it.second;

    double q = child->visits ? child->total_reward / child->visits : 0.0;
    double ucb =
        q + c * std::sqrt(std::log(visits + 1.0) / (child->visits + 1e-4));

    if (MctsDebugEnable) {
      std::cout << "[UCB] depth=" << child->path.size()
                << ", visits=" << child->visits << ", Q=" << q
                << ", UCB=" << ucb << "\n";
    }

    if (ucb > best_score) {
      best_score = ucb;
      best = child.get();
    }
  }

  return best;
}

void print_tree(const MCTSNode* node, int depth = 0) {
  if (!node)
    return;
  std::string indent(depth * 2, ' ');
  double q = node->visits ? node->total_reward / node->visits : 0.0;
  std::cout << indent << "[Node depth=" << depth << ", visits=" << node->visits
            << ", Q=" << q << ", best_reward=" << node->best_reward << "]\n";
  for (const auto& child_pair : node->children) {
    print_tree(child_pair.second.get(), depth + 1);
  }
}

TestCase run_mcts(std::mt19937& gen,
                  const MonteCarloSettings& settings,
                  const std::vector<recorder::Action>& actionSpace) {
  const auto initialReward = settings.objFunc(actionSpace).coverage;

  auto root = std::make_unique<MCTSNode>(TestCase{});

  for (int sim = 0; sim < settings.maxIter; ++sim) {
    std::cout << "\n[Iter " << sim << "]\n";
    MCTSNode* node = root.get();

    // === Selection ===
    std::unordered_set<recorder::Action, recorder::FuzzyActionHash,
                       recorder::FuzzyEqualPred>
        used_in_path;
    while (!node->children.empty() && node->fullyExpanded) {
      node = node->best_child_ucb(settings.ucb_C);
      used_in_path.insert(node->path.back());
    }

    // === Expansion ===
    MCTSNode* expanded = nullptr;
    for (const auto& a : actionSpace) {
      if (used_in_path.count(a) || node->children.count(a)) {
        continue;
      }

      TestCase new_path = node->path;
      new_path.push_back(a);
      if (!isValid(settings.objFunc, new_path)) {
        continue;
      }

      auto child = std::make_unique<MCTSNode>(new_path, node);
      expanded = child.get();
      node->children[a] = std::move(child);
      break;
    }

    if (!expanded) {
      std::cout << "[Expansion] No valid actions. Skipping sim.\n";
      node->fullyExpanded = true;
      continue;
    }

    // === Rollout ===
    TestCase candidate = expanded->path;
    const auto reward =
        rollout(gen, settings.objFunc, candidate, settings.maxRollback,
                settings.maxDepth, actionSpace);

    // === Backpropagation ===
    backpropagate(expanded, reward, candidate);
  }

  std::cout << "Initial size: " << actionSpace.size()
            << ", initial reward: " << initialReward << std::endl;
  std::cout << "Best path size: " << root->best_path.size()
            << ", best reward: " << root->best_reward << std::endl;

  std::cout << "\n=== Final MCTS Tree ===\n";
  print_tree(root.get());

  return root->best_path;
}

}  // namespace mcts
}  // namespace cider
