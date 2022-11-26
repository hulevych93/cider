#include "class_construct.h"

#include "recorder/actions_observer.h"
#include "recorder/details/lua/lua_params.h"

namespace gunit {
namespace models {

class ClassConstructImpl final {
 public:
  ClassConstructImpl() : _number(0), _condition(false) {}
  ClassConstructImpl(int number, bool condition)
      : _number(number), _condition(condition) {}
  ClassConstructImpl(const ClassConstructImpl& other)
      : _number(other._number), _condition(other._condition) {}
  ClassConstructImpl(ClassConstructImpl&& other)
      : _number(other._number), _condition(other._condition) {}

  void someMethod(int newNumber) { _number = newNumber; }

  ClassConstructImpl& operator=(const ClassConstructImpl& other) {
    _number = other._number;
    _condition = other._condition;
    return *this;
  }
  ClassConstructImpl& operator=(ClassConstructImpl&& other) {
    _number = other._number;
    _condition = other._condition;
    return *this;
  }

  bool operator==(const ClassConstructImpl& that) const {
    return _condition == that._condition && _number == that._number;
  }
  bool operator!=(const ClassConstructImpl& that) const {
    return (*this) != that;
  }

 private:
  int _number = 0;
  bool _condition = false;
};

ClassConstructImpl function_make_class_construct_obj_impl() {
  return ClassConstructImpl{4351, true};
}

ClassConstructImpl* function_make_class_construct_obj_ptr_impl() {
  return new ClassConstructImpl{411, true};
}

ClassConstructImpl function_test_class_construct_impl(
    const ClassConstructImpl& arg) {
  return arg;
}

ClassConstructImpl* function_test_class_construct_ptr_impl(
    ClassConstructImpl* arg) {
  return arg;
}

ClassConstruct::ClassConstruct() {
  try {
    _impl = std::make_shared<ClassConstructImpl>();
    GUNIT_NOTIFY_CONSTRUCTOR_NO_ARGS;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

ClassConstruct::ClassConstruct(std::shared_ptr<ClassConstructImpl>&& impl)
    : _impl(std::move(impl)) {}

ClassConstruct::ClassConstruct(const ClassConstruct& other) {
  try {
    _impl = std::make_shared<ClassConstructImpl>(*other._impl);
    GUNIT_NOTIFY_CONSTRUCTOR(other);
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

ClassConstruct::ClassConstruct(ClassConstruct&& other) {
  try {
    _impl = std::make_shared<ClassConstructImpl>(std::move(*other._impl));
    GUNIT_NOTIFY_CONSTRUCTOR(other);
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

ClassConstruct::ClassConstruct(int number, bool condition) {
  try {
    _impl = std::make_shared<ClassConstructImpl>(number, condition);
    GUNIT_NOTIFY_CONSTRUCTOR(number, condition);
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

void ClassConstruct::someMethod(const int newNumber) {
  try {
    _impl->someMethod(newNumber);
    GUNIT_NOTIFY_METHOD_NO_RETURN(newNumber);
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

ClassConstruct& ClassConstruct::operator=(const ClassConstruct& other) {
  try {
    GUNIT_NOTIFY_ASSIGNMENT(other);
    (*_impl) = *other._impl;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
  return *this;
}

ClassConstruct& ClassConstruct::operator=(ClassConstruct&& other) {
  try {
    GUNIT_NOTIFY_ASSIGNMENT(other);
    (*_impl) = std::move(*other._impl);
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
  return *this;
}

bool ClassConstruct::operator==(const ClassConstruct& that) const {
  try {
    return (*_impl) == *that._impl;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

bool ClassConstruct::operator!=(const ClassConstruct& that) const {
  try {
    return (*_impl) != *that._impl;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

ClassConstruct function_test_class_construct(const ClassConstruct& arg) {
  try {
    auto object = function_test_class_construct_impl(*arg._impl);
    auto result = ClassConstruct(std::make_shared<ClassConstructImpl>(object));
    GUNIT_NOTIFY_FREE_FUNCTION(result, arg);
    return result;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

ClassConstruct* function_test_class_construct_ptr(ClassConstruct* arg) {
  try {
    auto object = function_test_class_construct_ptr_impl(arg->_impl.get());
    auto result =
        new ClassConstruct(std::make_shared<ClassConstructImpl>(*object));
    GUNIT_NOTIFY_FREE_FUNCTION(result, arg);
    return result;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

ClassConstruct function_make_class_construct_obj() {
  try {
    auto object = function_make_class_construct_obj_impl();
    ClassConstruct result(std::make_shared<ClassConstructImpl>(object));
    GUNIT_NOTIFY_FREE_FUNCTION_NO_ARGS(result);
    return result;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

ClassConstruct* function_make_class_construct_obj_ptr() {
  try {
    auto object = function_make_class_construct_obj_ptr_impl();
    auto result =
        new ClassConstruct(std::make_shared<ClassConstructImpl>(*object));
    GUNIT_NOTIFY_FREE_FUNCTION_NO_ARGS(result);
    return result;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

}  // namespace models
}  // namespace gunit
