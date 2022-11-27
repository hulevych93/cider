#pragma once

#include <memory>

namespace gunit {
namespace models {

class SomeFinalClassImpl;

class SomeFinalClass final {
 public:
  SomeFinalClass();
  SomeFinalClass(int number, bool condition);
  SomeFinalClass(const SomeFinalClass& other);
  SomeFinalClass(SomeFinalClass&& other);

  void someMethod(int newNumber);

  SomeFinalClass& operator=(const SomeFinalClass& other);
  SomeFinalClass& operator=(SomeFinalClass&& other);

  bool operator==(const SomeFinalClass& that) const;
  bool operator!=(const SomeFinalClass& that) const;

  SomeFinalClass(std::shared_ptr<SomeFinalClassImpl>&& impl);

  // private:
  std::shared_ptr<SomeFinalClassImpl> _impl;
};

// function to test private constructed user data as parameter and return value
SomeFinalClass function_test_class_construct(const SomeFinalClass& arg);
SomeFinalClass* function_test_class_construct(SomeFinalClass* arg);

// function to test object construction inside api call
SomeFinalClass function_make_class_construct_obj();
SomeFinalClass* function_make_class_construct_obj_ptr();

}  // namespace models
}  // namespace gunit
