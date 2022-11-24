#pragma once

namespace gunit {
namespace models {

class ClassConstruct final {
 public:
  ClassConstruct();
  ClassConstruct(int number, bool condition);
  ClassConstruct(const ClassConstruct& other);
  ClassConstruct(ClassConstruct&& other);

  void someMethod(int newNumber);

  ClassConstruct& operator=(const ClassConstruct& other);
  ClassConstruct& operator=(ClassConstruct&& other);

  bool operator==(const ClassConstruct& that) const;
  bool operator!=(const ClassConstruct& that) const;

 private:
  int _number = 0;
  bool _condition = false;
};

// function to test private constructed user data as parameter and return value
ClassConstruct function_test_class_construct(const ClassConstruct& arg);

}  // namespace models
}  // namespace gunit
