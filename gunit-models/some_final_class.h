#pragma once

#include "models/some_final_class.h"

namespace gunit {
namespace models {
namespace generated {

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

  SomeFinalClass(std::shared_ptr<models::SomeFinalClass>&& impl);

  // private:
  std::shared_ptr<models::SomeFinalClass> _impl;
};

SomeFinalClass function_test_class_construct(const SomeFinalClass& arg);
SomeFinalClass* function_test_class_construct(SomeFinalClass* arg);

SomeFinalClass function_make_class_construct_obj();
SomeFinalClass* function_make_class_construct_obj_ptr();

}  // namespace generated
}  // namespace models
}  // namespace gunit
