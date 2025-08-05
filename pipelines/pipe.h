// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "coverage/coverage.h"

#include "results.h"

namespace cider {
namespace pipelines {

class Pipeline;

class Pipe {
 public:
  virtual ~Pipe() = default;

  virtual bool process(const std::string& metadata,
                       const std::string& libName,
                       const cider::Cmd& cmd) = 0;

  virtual std::string getLetter() const = 0;

 protected:
  friend class Pipeline;

  void setOwner(Pipeline* owner) { _owner = owner; }

  bool pushResult(const std::string& libName,
                  const cider::Cmd& cmdl,
                  const std::string& methodName,
                  Result result);

  const Input& getInput() const;
  const Results& getResults() const;

  Results& getMutableResults();

  void clearData(const std::string& methodName);

 private:
  Pipeline* _owner = nullptr;
};

}  // namespace pipelines
}  // namespace cider
