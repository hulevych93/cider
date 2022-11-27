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

class StringInterfaceImpl;

class StringInterface : public SomeInterface {
 public:
  StringInterface(const char* str);
  bool isEmpty() const override;

  StringInterface(std::shared_ptr<StringInterfaceImpl> impl);
  void setImpl(std::shared_ptr<StringInterfaceImpl> impl);

 private:
  std::shared_ptr<StringInterfaceImpl> _impl;
};

class OtherStringInterfaceImpl;

class OtherStringInterface final : public StringInterface {
 public:
  OtherStringInterface(bool empty);

  OtherStringInterface(std::shared_ptr<OtherStringInterfaceImpl> impl);
  void setImpl(std::shared_ptr<OtherStringInterfaceImpl> impl);

 private:
  std::shared_ptr<OtherStringInterfaceImpl> _impl;
};

SomeInterface* makeSomeInterface(const char* str);

}  // namespace models
}  // namespace gunit
