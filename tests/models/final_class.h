#pragma once

#include <memory>

namespace gunit {
namespace models {

class FinalClass final {
 public:
  FinalClass();
  FinalClass(int number, bool condition);
  FinalClass(const FinalClass& other);
  FinalClass(FinalClass&& other);

  void someMethod(int newNumber);

  FinalClass& operator=(const FinalClass& other);
  FinalClass& operator=(FinalClass&& other);

  bool operator==(const FinalClass& that) const;
  bool operator!=(const FinalClass& that) const;

 private:
  int _number = 0;
  bool _condition = false;
};

// function to test private constructed user data as parameter and return value
FinalClass function_test_class_construct(const FinalClass& arg);
FinalClass* function_test_class_construct_same_pointer_return(FinalClass* arg);
FinalClass* function_test_class_construct(FinalClass* arg);

// function to test object construction inside api call
FinalClass function_make_class_construct_obj();
FinalClass* function_make_class_construct_obj_ptr();

}  // namespace models
}  // namespace gunit
