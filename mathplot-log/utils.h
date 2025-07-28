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

constexpr const char* ColorCodes[] = {
    "k",  // black
    "b",  // blue
    "r",  // red
    "g",  // green
    "c",  // cyan
    "m",  // magenta
    "y",  // yellow
};

struct BoxStats {
  double q1, median, q3, iqr, mean = 0;
  double lower_whisker, upper_whisker;
  std::vector<double> outliers;
};

BoxStats compute_box(std::vector<double> data);

double compute_average(const std::vector<size_t>& vec);

void computeMeanAndStd(const std::vector<std::vector<double>>& values,
                       std::vector<double>& meanOut,
                       std::vector<double>& stdOut);

std::string ensurePngExtension(const std::string& path);
std::string ensureBinExtension(const std::string& path);

std::string ensurePath(const std::string& logDir,
                       const std::string& logFileName);

double mann_whitney_u(const std::vector<double>& group1,
                      const std::vector<double>& group2,
                      const std::string& alternative = "two-sided");

void applyPublicationStyle();

}  // namespace cider
