// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

namespace cider {

constexpr const char* LineStyles[] = {
    "--",  // dashed line
    "-",   // solid line
    "-.",  // dash-dot line
    ":",   // dotted line
};

constexpr const char* MarkerStyles[] = {
    "o",
    "s",
    "D",
    "*",
};

std::string ensureExtension(const std::string& path, const std::string& ext);

std::string ensurePath(const std::string& logDir,
                       const std::string& logFileName);

double kruskal_wallis(const std::vector<std::vector<double>>& groups);

double mann_whitney_u(const std::vector<double>& group1,
                      const std::vector<double>& group2,
                      const std::string& alternative = "two-sided");

void applyPublicationStyle();

void rotateXTicks90();

void tightLighout();

void setAxisPolicy();

void disableFrame();

void makeLegentByGroups(const std::vector<std::string>& groups,
                        const std::vector<double>& positions);

inline std::vector<std::string> getColorGroups() {
  std::vector<std::string> groups;
  groups.emplace_back("DSL");
  groups.emplace_back("GR");
  groups.emplace_back("MCTS");
  groups.emplace_back("QLEG");
  groups.emplace_back("QLB");
  groups.emplace_back("QLEG+DSL");
  groups.emplace_back("QLB+DSL");
  return groups;
}

inline std::string getColorByLabel(const std::string& label) {
  if (label.rfind("DSL", 0) == 0)
    return "#1f77b4";  // синій
  if (label.rfind("GR", 0) == 0)
    return "#2ca02c";  // зелений
  if (label.rfind("MCTS", 0) == 0)
    return "#1f9e89";  // бірюзово-зелений
  if (label.rfind("QLEG", 0) == 0 && label.find("+DSL") == std::string::npos)
    return "#9467bd";  // фіолетовий
  if (label.rfind("QLB", 0) == 0 && label.find("+DSL") == std::string::npos)
    return "#c49c94";  // коричневий
  if (label.rfind("QLEG", 0) == 0 && label.find("+DSL") != std::string::npos)
    return "#17becf";  // бірюзовий
  if (label.rfind("QLB", 0) == 0 && label.find("+DSL") != std::string::npos)
    return "#ff7f0e";  // помаранчевий
  return "#7f7f7f";    // сірий дефолт
}

}  // namespace cider
