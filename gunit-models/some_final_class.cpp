#include "some_final_class.h"

#include "recorder/actions_observer.h"

namespace gunit {
namespace models {
namespace generated {

SomeFinalClass::SomeFinalClass() {
  try {
    _impl = std::make_shared<models::SomeFinalClass>();
    GUNIT_NOTIFY_CONSTRUCTOR_NO_ARGS;
  } catch (const std::exception&) {
    // ex.what() notify
    throw;
  } catch (...) {
    // NOTIFY some expection
    throw;
  }
}

SomeFinalClass::SomeFinalClass(std::shared_ptr<models::SomeFinalClass>&& impl)
    : _impl(std::move(impl)) {}

SomeFinalClass::SomeFinalClass(const SomeFinalClass& other) {
  try {
    _impl = std::make_shared<models::SomeFinalClass>(*other._impl);
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
    _impl = std::make_shared<models::SomeFinalClass>(number, condition);
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
    auto object = models::function_test_class_construct(*arg._impl);
    auto result = SomeFinalClass(std::make_shared<models::SomeFinalClass>(object));
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
    auto object = models::function_test_class_construct(arg->_impl.get());
    auto result =
        new SomeFinalClass(std::make_shared<models::SomeFinalClass>(*object));
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
    auto object = models::function_make_class_construct_obj();
    SomeFinalClass result(std::make_shared<models::SomeFinalClass>(object));
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
    auto object = models::function_make_class_construct_obj_ptr();
    auto result =
        new SomeFinalClass(std::make_shared<models::SomeFinalClass>(*object));
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

} // namespace generated
} // namespace models
} // namespace gunit
