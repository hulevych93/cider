#pragma once

#include <memory>
#include <string>

namespace cider {
namespace models {

class BaseClass {
 public:
  BaseClass(const char* str);

  std::string sayHello() const;

 private:
  std::string _impl;
};

class DerivedClass final : public BaseClass {
 public:
  DerivedClass(const char* str);

  std::string sayGoodbye(int times) const;

 private:
  std::string _impl;
};

}  // namespace models
}  // namespace cider
