#pragma once

#include <memory>
#include <string>

namespace gunit {
namespace models {

class SomeInterface {
 public:
  virtual ~SomeInterface() = default;
  virtual bool isEmpty() const = 0;
};

class StringInterface : public SomeInterface {
 public:
  StringInterface(const char* str);
  bool isEmpty() const override;

 private:
  std::string _impl;
};

class OtherStringInterface final : public StringInterface {
 public:
  OtherStringInterface(bool empty);
};

SomeInterface* makeSomeInterface(const char* str);

}  // namespace models
}  // namespace gunit