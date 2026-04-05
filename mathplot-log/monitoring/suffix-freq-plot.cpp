// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "suffix-freq-plot.h"

#include <assert.h>

#include <tlog.h>

#include <matplotlibcpp.h>

#include "serialization/deserializer.h"
#include "serialization/serializer.h"

namespace plt = matplotlibcpp;

#include "mathplot-log/utils.h"

namespace cider {
namespace mathplot {

namespace {

std::string getYAxisName() {
#ifdef ENG_NAMES
  return "Relative frequency of state–action\npair occurrences in the Q-table, "
         "a.u.";
#else
  return "Відносна частота знаходження записів\nу Q-таблиці, ум. од.";
#endif
}

std::string getXAxisName() {
#ifdef ENG_NAMES
  return "TC Suffix length k";
#else
  return "Довжина суфікса ТС η";
#endif
}

}  // namespace

SuffixFreqPlot::SuffixFreqPlot(const std::string& logDir,
                               const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

SuffixFreqPlot::~SuffixFreqPlot() {
  save();

  plot();
  plt::save(ensureExtension(m_path, ".eps"), 1200);
}

void SuffixFreqPlot::log(int suffix) const {
  if (suffix >= suffixFreq_.size())
    suffixFreq_.resize(suffix + 1, 0.0);
  suffixFreq_[suffix] += 1.0;
}

void SuffixFreqPlot::save() {
  try {
    serialization::Serializer serializer;
    serializer << suffixFreq_;
    serializer.save(ensureExtension(m_path, ".bin"));
  } catch (...) {
    tlog_info << "Graph serialization failed : "
              << ensureExtension(m_path, ".bin") << std::endl;
  }
}

bool SuffixFreqPlot::load(const std::string& logFile) {
  try {
    auto path = logFile;
    if (path.empty()) {
      path = m_path;
    }

    serialization::Deserializer deserializer(ensureExtension(path, ".bin"));
    deserializer >> suffixFreq_;
  } catch (const std::exception& e) {
    tlog_info << e.what() << std::endl;
    return false;
  }
  return true;
}
void SuffixFreqPlot::plot() const {
  if (suffixFreq_.empty())
    return;

  std::vector<double> x(suffixFreq_.size()), y(suffixFreq_.size()),
      err(suffixFreq_.size());
  double total = std::accumulate(suffixFreq_.begin(), suffixFreq_.end(), 0.0);
  if (total <= 0.0)
    return;

  for (size_t i = 0; i < suffixFreq_.size(); ++i) {
    x[i] = static_cast<double>(i);
    y[i] = suffixFreq_[i] / total;
    err[i] = std::sqrt(suffixFreq_[i]) / total;  // статистична похибка
  }

  plt::clf();
  plt::plot(x, y,
            {{"color", "#404040"},
             {"linewidth", "1.4"},
             {"marker", "o"},
             {"markerfacecolor", "#000000"},
             {"markeredgecolor", "#000000"},
             {"markersize", "4"},
             {"linestyle", "--"}});

  // Похибка (error bars)
  plt::errorbar(x, y, err,
                {{"fmt", "none"},
                 {"ecolor", "red"},
                 {"elinewidth", "0.8"},
                 {"capsize", "3"}});

  plt::xlabel(getXAxisName());
  plt::ylabel(getYAxisName());

  applyPublicationStyle();

  plt::grid(true);
  plt::tight_layout();
}

}  // namespace mathplot
}  // namespace cider
