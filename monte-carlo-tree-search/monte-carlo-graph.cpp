// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "monte-carlo-graph.h"

#include "synthesis/synthesis.h"

#include <tlog.h>
#include <array>
#include <iomanip>
#include <limits>
#include <ostream>
#include <sstream>
#include <string>

namespace cider {
namespace mcts {

// утиліта для Action → строка
std::string actionToGenericRepro(const recorder::Action& action) {
  std::stringstream os;
  std::visit(
      [&os](auto&& value) {
        using T = std::decay_t<decltype(value)>;

        auto printParams = [&os](const auto& args) {
          os << "(";
          for (size_t i = 0; i < args.size(); ++i) {
            os << "_";
            if (i + 1 != args.size())
              os << ", ";
          }
          os << ")";
        };

        if constexpr (std::is_same_v<T, cider::recorder::Function>) {
          os << value.name;
          printParams(value.params);
        } else if constexpr (std::is_same_v<T, cider::recorder::ClassMethod>) {
          os << "obj." << value.method.name;
          printParams(value.method.params);
        } else if constexpr (std::is_same_v<T,
                                            cider::recorder::ClassBinaryOp>) {
          std::string opStr =
              (value.opName == cider::recorder::BinaryOpType::Assignment) ? "="
                                                                          : "?";
          os << "obj " << opStr;
        } else if constexpr (std::is_same_v<T, cider::recorder::ClassUnaryOp>) {
          std::string opStr =
              (value.opName == cider::recorder::UnaryOpType::Minus) ? "-" : "?";
          os << opStr << "obj";
        } else if constexpr (std::is_same_v<T,
                                            cider::recorder::ClassDestructor>) {
          os << "destroy(obj)";
        } else {
          static_assert(!sizeof(T), "Unsupported Action type");
        }
      },
      action);
  return os.str();
}

// ===== кольоровий маппінг від min..max reward =====

std::string lerpColor(double t,
                      const std::array<int, 3>& c1,
                      const std::array<int, 3>& c2) {
  auto clamp = [](int x) { return std::max(0, std::min(255, x)); };
  int r = clamp((int)(c1[0] + t * (c2[0] - c1[0])));
  int g = clamp((int)(c1[1] + t * (c2[1] - c1[1])));
  int b = clamp((int)(c1[2] + t * (c2[2] - c1[2])));
  char buf[16];
  snprintf(buf, sizeof(buf), "#%02x%02x%02x", r, g, b);
  return std::string(buf);
}

std::string nodeColorNormalized(double reward,
                                double minReward,
                                double maxReward) {
  if (maxReward - minReward < 1e-9) {
    return "#bbbbbb";  // всі однакові
  }
  double norm = (reward - minReward) / (maxReward - minReward);

  // градієнт: червоний (#fc9272) → жовтий (#fee391) → зелений (#a1d99b)
  if (norm < 0.5) {
    return lerpColor(norm * 2.0, {252, 146, 114}, {254, 227, 145});
  } else {
    return lerpColor((norm - 0.5) * 2.0, {254, 227, 145}, {161, 217, 155});
  }
}

// ===== обхід для пошуку min/max =====
void findRewardRange(const MCTSNode* node, double& minR, double& maxR) {
  if (!node)
    return;
  minR = std::min(minR, node->best_reward);
  maxR = std::max(maxR, node->best_reward);
  for (const auto& [a, child] : node->children) {
    findRewardRange(child.get(), minR, maxR);
  }
}

// ===== рекурсивний експорт вузлів =====
void export_tree_dot_pretty(const MCTSNode* node,
                            std::ostream& os,
                            double minReward,
                            double maxReward,
                            const std::string& parentId = "",
                            int depth = 0,
                            int& counter = *(new int(0))) {
  if (!node)
    return;

  std::string nodeId = "n" + std::to_string(counter++);
  double q = node->visits ? node->total_reward / node->visits : 0.0;

  // верхній рядок = Action (або ROOT)
  std::ostringstream label;
  std::string fill = node->parent ? nodeColorNormalized(node->best_reward,
                                                        minReward, maxReward)
                                  : "#9ecae1:#deebf7";

  label << "<TABLE BORDER=\"1\" CELLBORDER=\"0\" CELLSPACING=\"0\" "
           "CELLPADDING=\"4\" BGCOLOR=\""
        << fill << "\">";

  if (!node->path.empty()) {
    const auto& lastAction = node->path.back();
    label << "<TR><TD ALIGN=\"CENTER\"><B>" << actionToGenericRepro(lastAction)
          << "</B></TD></TR>";
  } else {
    label << "<TR><TD ALIGN=\"CENTER\"><B>&lt;ROOT&gt;</B></TD></TR>";
  }

  // нижній рядок = статистика
  std::ostringstream stats;
  stats << "visits=" << node->visits << " | Q=" << std::fixed
        << std::setprecision(2) << q << " | best=" << node->best_reward;

  label << "<TR><TD ALIGN=\"LEFT\">" << stats.str() << "</TD></TR>";
  label << "</TABLE>";

  // вузол
  os << "  " << nodeId << " [shape=plaintext, label=<" << label.str()
     << ">];\n";

  // ребро до батька
  if (!parentId.empty()) {
    double penWidth = 1.0 + std::log1p(node->visits);
    os << "  " << parentId << " -> " << nodeId << " [label=\"" << node->visits
       << "\", "
       << "fontname=\"Helvetica\", fontsize=9, fontcolor=\"#444444\", "
       << "color=\"#555555\", arrowsize=0.7, arrowhead=vee, "
       << "penwidth=" << penWidth << "];\n";
  }

  // діти
  for (const auto& [action, child] : node->children) {
    export_tree_dot_pretty(child.get(), os, minReward, maxReward, nodeId,
                           depth + 1, counter);
  }
}

// ===== обгортка =====
void export_tree_dot_wrapper(const MCTSNode* root, std::ostream& os) {
  double minReward = std::numeric_limits<double>::infinity();
  double maxReward = -std::numeric_limits<double>::infinity();
  findRewardRange(root, minReward, maxReward);

  os << "digraph MCTS {\n";
  os << "  rankdir=TB;\n";
  os << "  graph [nodesep=0.5, ranksep=0.7];\n";
  os << "  node [fontname=\"Helvetica\"];\n";
  int counter = 0;
  export_tree_dot_pretty(root, os, minReward, maxReward, "", 0, counter);
  os << "}\n";
}

}  // namespace mcts
}  // namespace cider
