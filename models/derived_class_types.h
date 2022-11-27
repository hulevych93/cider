#pragma once

#include <memory>
#include <string>

namespace gunit {
namespace models {

class SomeBaseImpl;

class SomeBase {
 public:
  SomeBase(const char* str);

  std::string sayHello() const;

  SomeBase(std::shared_ptr<SomeBaseImpl> impl);
  void setImpl(std::shared_ptr<SomeBaseImpl> impl);

 private:
  std::shared_ptr<SomeBaseImpl> _impl;
};

class SomeDerivedImpl;

class SomeDerived final : public SomeBase {
 public:
  SomeDerived(const char* str);

  std::string sayGoodbye(int times) const;

  SomeDerived(std::shared_ptr<SomeDerivedImpl> impl);
  void setImpl(std::shared_ptr<SomeDerivedImpl> impl);

 private:
  std::shared_ptr<SomeDerivedImpl> _impl;
};

}  // namespace models
}  // namespace gunit
