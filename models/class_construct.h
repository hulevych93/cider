#pragma once

#include <memory>

namespace gunit {
namespace models {

class ClassConstructImpl;

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

  ClassConstruct(std::shared_ptr<ClassConstructImpl>&& impl);

  // private:
  std::shared_ptr<ClassConstructImpl> _impl;
};

// function to test private constructed user data as parameter and return value
ClassConstruct function_test_class_construct(const ClassConstruct& arg);
ClassConstruct* function_test_class_construct_ptr(ClassConstruct* arg);

// function to test object construction inside api call
ClassConstruct function_make_class_construct_obj();
ClassConstruct* function_make_class_construct_obj_ptr();

}  // namespace models
}  // namespace gunit
