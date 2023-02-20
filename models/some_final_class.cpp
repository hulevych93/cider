#include "some_final_class.h"

#include "recorder/actions_observer.h"

namespace gunit {
namespace models {

class SomeFinalClassImpl final {
 public:
  SomeFinalClassImpl() : _number(0), _condition(false) {}
  SomeFinalClassImpl(int number, bool condition)
      : _number(number), _condition(condition) {}
  SomeFinalClassImpl(const SomeFinalClassImpl& other)
      : _number(other._number), _condition(other._condition) {}
  SomeFinalClassImpl(SomeFinalClassImpl&& other)
      : _number(other._number), _condition(other._condition) {}

  void someMethod(int newNumber) { _number = newNumber; }

  SomeFinalClassImpl& operator=(const SomeFinalClassImpl& other) {
    _number = other._number;
    _condition = other._condition;
    return *this;
  }
  SomeFinalClassImpl& operator=(SomeFinalClassImpl&& other) {
    _number = other._number;
    _condition = other._condition;
    return *this;
  }

  bool operator==(const SomeFinalClassImpl& that) const {
    return _condition == that._condition && _number == that._number;
  }
  bool operator!=(const SomeFinalClassImpl& that) const {
    return (*this) != that;
  }

 private:
  int _number = 0;
  bool _condition = false;
};

SomeFinalClassImpl function_make_class_construct_obj_impl() {
  return SomeFinalClassImpl{4351, true};
}

SomeFinalClassImpl* function_make_class_construct_obj_ptr_impl() {
  return new SomeFinalClassImpl{411, true};
}

SomeFinalClassImpl function_test_class_construct_impl(
    const SomeFinalClassImpl& arg) {
  return arg;
}

SomeFinalClassImpl* function_test_class_construct_impl(
    SomeFinalClassImpl* arg) {
  return arg;
}

SomeFinalClass::SomeFinalClass() {
  try {
    _impl = std::make_shared<SomeFinalClassImpl>();
    GUNIT_NOTIFY_CONSTRUCTOR_NO_ARGS;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

SomeFinalClass::SomeFinalClass(std::shared_ptr<SomeFinalClassImpl>&& impl)
    : _impl(std::move(impl)) {}

SomeFinalClass::SomeFinalClass(const SomeFinalClass& other) {
  try {
    _impl = std::make_shared<SomeFinalClassImpl>(*other._impl);
    GUNIT_NOTIFY_CONSTRUCTOR(other._impl.get());
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

SomeFinalClass::SomeFinalClass(SomeFinalClass&& other) {
  try {
    _impl = std::move(other._impl);
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

SomeFinalClass::SomeFinalClass(int number, bool condition) {
  try {
    _impl = std::make_shared<SomeFinalClassImpl>(number, condition);
    GUNIT_NOTIFY_CONSTRUCTOR(number, condition);
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

void SomeFinalClass::someMethod(const int newNumber) {
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

SomeFinalClass& SomeFinalClass::operator=(const SomeFinalClass& other) {
  try {
    GUNIT_NOTIFY_ASSIGNMENT(other._impl.get());
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

SomeFinalClass& SomeFinalClass::operator=(SomeFinalClass&& other) {
  try {
    GUNIT_NOTIFY_ASSIGNMENT(other._impl.get());
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

bool SomeFinalClass::operator==(const SomeFinalClass& that) const {
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

bool SomeFinalClass::operator!=(const SomeFinalClass& that) const {
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

SomeFinalClass function_test_class_construct(const SomeFinalClass& arg) {
  try {
    auto object = function_test_class_construct_impl(*arg._impl);
    auto result = SomeFinalClass(std::make_shared<SomeFinalClassImpl>(object));
    GUNIT_NOTIFY_FREE_FUNCTION(result._impl.get(), arg._impl.get());
    return result;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

SomeFinalClass* function_test_class_construct(SomeFinalClass* arg) {
  try {
    auto object = function_test_class_construct_impl(arg->_impl.get());
    auto result =
        new SomeFinalClass(std::make_shared<SomeFinalClassImpl>(*object));
    GUNIT_NOTIFY_FREE_FUNCTION(result->_impl.get(), arg->_impl.get());
    return result;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

SomeFinalClass function_make_class_construct_obj() {
  try {
    auto object = function_make_class_construct_obj_impl();
    SomeFinalClass result(std::make_shared<SomeFinalClassImpl>(object));
    GUNIT_NOTIFY_FREE_FUNCTION_NO_ARGS(result._impl.get());
    return result;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

SomeFinalClass* function_make_class_construct_obj_ptr() {
  try {
    auto object = function_make_class_construct_obj_ptr_impl();
    auto result =
        new SomeFinalClass(std::make_shared<SomeFinalClassImpl>(*object));
    GUNIT_NOTIFY_FREE_FUNCTION_NO_ARGS(result->_impl.get());
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
