// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "recorder/details/params.h"

#include <memory>
#include <string>

namespace cider {
namespace recorder {

class CodeSink;

namespace lua {

struct ParamVisitor {
  explicit ParamVisitor(const std::string& moduleName);

  std::string operator()(const Nil&) const;
  std::string operator()(bool value) const;
  std::string operator()(double value) const;
  std::string operator()(const char* value) const;
  std::string operator()(const std::string& value) const;
  std::string operator()(const std::wstring& value) const;

  std::string operator()(const IntegerType& value) const;

  std::string operator()(char value) const;
  std::string operator()(short value) const;
  std::string operator()(int value) const;
  std::string operator()(long value) const;
  std::string operator()(long long value) const;
  std::string operator()(unsigned char value) const;
  std::string operator()(unsigned short value) const;
  std::string operator()(unsigned int value) const;
  std::string operator()(unsigned long value) const;
  std::string operator()(unsigned long long value) const;

 protected:
  std::string _moduleName;
};

class UserDataParamVisitor final : public ParamVisitor {
 public:
  UserDataParamVisitor(const std::string& moduleName, CodeSink& sink);

  using ParamVisitor::operator();

  std::string operator()(const UserDataValueParamPtr& value) const;

 private:
  CodeSink& _sink;
};

std::string produceParamCode(const std::string& moduleName,
                             const Param& param,
                             CodeSink& sink);

}  // namespace lua
}  // namespace recorder
}  // namespace cider
