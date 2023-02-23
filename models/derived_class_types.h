#pragma once

#include <memory>
#include <string>

namespace gunit {
namespace models {

class SomeBase {
 public:
  SomeBase(const char* str);

  std::string sayHello() const;

 private:
  std::string _impl;
};

class SomeDerived final : public SomeBase {
 public:
  SomeDerived(const char* str);

  std::string sayGoodbye(int times) const;

private:
  std::string _impl;
};

}  // namespace models
}  // namespace gunit
