#pragma once

#include "models/derived_class_types.h"

namespace gunit {
namespace models {
namespace generated {

class SomeBase {
 public:
  SomeBase(const char* str);

  std::string sayHello() const;

  SomeBase(std::shared_ptr<models::SomeBase> impl);
  void setImpl(std::shared_ptr<models::SomeBase> impl);

 private:
  std::shared_ptr<models::SomeBase> _impl;
};

class SomeDerived final : public SomeBase {
 public:
  SomeDerived(const char* str);

  std::string sayGoodbye(int times) const;

  SomeDerived(std::shared_ptr<models::SomeDerived> impl);
  void setImpl(std::shared_ptr<models::SomeDerived> impl);

 private:
  std::shared_ptr<models::SomeDerived> _impl;
};

} // namespace generated
} // namespace models
} // namespace gunit
