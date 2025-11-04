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

inline std::vector<std::string> getColorGroups(const std::vector<std::string>& labels) {

    static const std::vector<std::string> knownGroups = {
        "DSL",
        "GR",
        "MCTS",
        "QLEG",
        "QLB",
        "QLB-M",
        "QLB-MD",
    };

    std::vector<std::string> foundGroups;

    for (const auto& group : knownGroups) {
        // Check if any label starts with this group name
        bool exists = std::any_of(labels.begin(), labels.end(), [&](const std::string& label) {
            return label.rfind(group, 0) == 0;  // prefix match
        });

        if (exists)
            foundGroups.push_back(group);
    }

    return foundGroups;
}

inline std::string getColorByLabel(const std::string& label) {
    // --- Base groups ---
    if (label.rfind("DSL", 0) == 0)
        return "#595959";  // light gray
    if (label.rfind("GR", 0) == 0)
        return "#b0b0b0";  // dark gray
    if (label.rfind("MCTS", 0) == 0)
        return "#1f9e89";  // light green

    // --- QLEG / QLB without DSL combination ---
    if (label.rfind("QLEG", 0) == 0 && label.find("+DSL") == std::string::npos)
        return "#9467bd";  // purple
    if (label.rfind("QLB", 0) == 0 && label.find("+DSL") == std::string::npos)
        return "#c49c94";  // brown-red

    // --- QLEG / QLB combined with DSL ---
    if (label.rfind("QLEG", 0) == 0 && label.find("+DSL") != std::string::npos)
        return "#17becf";  // blue
    if (label.rfind("QLB", 0) == 0 && label.find("+DSL") != std::string::npos)
        return "#ff7f0e";  // orange

    // --- Variants with -M or -MD (explicit handling) ---
    if (label.rfind("QLB-M", 0) == 0 && label.find("-MD") == std::string::npos)
        return "#9467bd";  // purple (QLB-M)
    if (label.rfind("QLB-MD", 0) == 0)
        return "#c49c94";  // brown-red (QLB-MD)

    // --- Default fallback color ---
    return "#7f7f7f";  // gray
}

inline std::string getMarkerByConfig(const std::string& label) {
  for (int conf = 1; conf <= 4; ++conf) {
    if (label.find(std::to_string(conf)) != std::string::npos) {
      switch (conf) {
        case 1:
          return "o";
        case 2:
          return "s";
        case 3:
          return "^";
      }
    }
  }
  return "D";
}

}  // namespace cider
