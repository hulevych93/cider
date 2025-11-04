// Copyright (C) 2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "suffix-exist-rate-plot.h"

#include <assert.h>
#include <matplotlibcpp.h>
#include <tlog.h>

#include "serialization/deserializer.h"
#include "serialization/serializer.h"

namespace plt = matplotlibcpp;

#include "mathplot-log/utils.h"

namespace cider {
namespace mathplot {

namespace {

std::string getYAxisName() {
#ifdef ENG_NAMES
  return "Fraction of known suffixes";
#else
  return "Частка знайдених записів\nу Q-таблиці, ум. од.";
#endif
}

std::string getXAxisName() {
#ifdef ENG_NAMES
  return "Configuration (λ)";
#else
  return "Коефіцієнт згасання λ";
#endif
}

}  // namespace

SuffixExistRatePlot::SuffixExistRatePlot(const std::string& logDir,
                                         const std::string& logFileName)
    : m_path(ensurePath(logDir, logFileName)) {}

void SuffixExistRatePlot::finalize() {
  save();
  plot();
  plt::save(ensureExtension(m_path, ".eps"), 1200);
}

void SuffixExistRatePlot::next(const double configName) {
  _current = std::addressof(suffixExist_[configName]);
}

void SuffixExistRatePlot::log(size_t suffix, bool found) const {
  assert(_current != nullptr);

  if (suffix > 5)
    return;

  _current->push_back(found ? 1.0 : 0.0);

  if ((_updateCounter++ % 50) == 0) {
    plot();
  }
}

void SuffixExistRatePlot::save() {
  try {
    serialization::Serializer serializer;
    serializer << suffixExist_;
    serializer.save(ensureExtension(m_path, ".bin"));
  } catch (...) {
    tlog_info << "Graph serialization failed : "
              << ensureExtension(m_path, ".bin") << std::endl;
  }
}

bool SuffixExistRatePlot::load(const std::string& logFile) {
  try {
    std::string path = logFile;
    if (path.empty())
      path = m_path;

    serialization::Deserializer deserializer(ensureExtension(path, ".bin"));
    deserializer >> suffixExist_;
  } catch (const std::exception& e) {
    tlog_info << e.what() << std::endl;
    return false;
  }
  return true;
}

std::map<double, double> SuffixExistRatePlot::meanRates() const {
  std::map<double, double> res;
  for (auto it = suffixExist_.begin(); it != suffixExist_.end(); ++it) {
    const double cfg = it->first;
    const std::vector<double>& vals = it->second;
    if (vals.empty())
      continue;

    double sum = 0.0;
    for (size_t i = 0; i < vals.size(); ++i)
      sum += vals[i];
    res[cfg] = sum / vals.size();
  }
  return res;
}

void SuffixExistRatePlot::plot() const {
  if (suffixExist_.empty())
    return;

  plt::clf();

  std::vector<double> x, y, err;
  for (auto it = suffixExist_.begin(); it != suffixExist_.end(); ++it) {
    const double cfg = it->first;
    const std::vector<double>& vals = it->second;
    if (vals.empty())
      continue;

    double sum = 0.0;
    for (size_t i = 0; i < vals.size(); ++i)
      sum += vals[i];
    double mean = sum / vals.size();

    double var = 0.0;
    for (size_t i = 0; i < vals.size(); ++i)
      var += (vals[i] - mean) * (vals[i] - mean);
    var /= vals.size();

    double stddev = std::sqrt(var);
    double stderr = stddev / std::sqrt(static_cast<double>(vals.size()));

    x.push_back(cfg);
    y.push_back(mean);
    err.push_back(stderr * 0.5);
  }

  if (x.empty())
    return;

  plt::xlabel(getXAxisName(), {{"fontsize", "12"}});
  plt::ylabel(getYAxisName(), {{"fontsize", "12"}});

  plt::plot(x, y,
            {{"color", "#404040"},
             {"linewidth", "1.4"},
             {"marker", "o"},
             {"markerfacecolor", "#000000"},
             {"markeredgecolor", "#000000"},
             {"markersize", "4"},
             {"linestyle", "--"}});

  plt::errorbar(x, y, err,
                {{"fmt", "none"},
                 {"ecolor", "red"},
                 {"elinewidth", "0.8"},
                 {"capsize", "3"}});

  applyPublicationStyle();

  plt::grid(true);
  plt::tight_layout();
  plt::pause(0.01);
}

}  // namespace mathplot
}  // namespace cider
