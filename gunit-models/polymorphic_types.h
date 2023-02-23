#pragma once

#include "models/polymorphic_types.h"

namespace gunit {
namespace models {
namespace generated {

class SomeInterface {
 public:
  virtual ~SomeInterface() = default;
  virtual bool isEmpty() const;

  SomeInterface() = default;
  SomeInterface(std::shared_ptr<models::SomeInterface> impl);
  void setImpl(std::shared_ptr<models::SomeInterface> impl);

private:
  std::shared_ptr<models::SomeInterface> _impl;
};

class StringInterface : public SomeInterface {
 public:
  StringInterface(const char* str);
  bool isEmpty() const override;

  StringInterface(std::shared_ptr<models::StringInterface> impl);
  void setImpl(std::shared_ptr<models::StringInterface> impl);

 private:
  std::shared_ptr<models::StringInterface> _impl;
};

class OtherStringInterface final : public StringInterface {
 public:
  OtherStringInterface(bool empty);

  OtherStringInterface(std::shared_ptr<models::OtherStringInterface> impl);
  void setImpl(std::shared_ptr<models::OtherStringInterface> impl);

 private:
  std::shared_ptr<models::OtherStringInterface> _impl;
};

SomeInterface* makeSomeInterface(const char* str);

} // namespace generated
} // namespace models
} // namespace gunit
