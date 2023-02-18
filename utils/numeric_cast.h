#pragma once

#include <exception>
#include <type_traits>

#include <boost/numeric/conversion/cast.hpp>

namespace gunit {
class BadNumCast final : public std::exception {
 public:
  explicit BadNumCast(const char* what) : _error("BadNumCast: ") {
    _error += what;
  }

  const char* what() const noexcept override { return _error.c_str(); }

 private:
  std::string _error;
};

template <typename To,
          typename From,
          std::enable_if_t<std::is_same<To, From>::value>* = nullptr>
To numCast(const From from) {
  return from;
}

template <typename To,
          typename From,
          std::enable_if_t<!std::is_same<To, From>::value>* = nullptr>
To numCast(const From from) {
  try {
    return boost::numeric_cast<To>(from);
  } catch (const boost::bad_numeric_cast& exception) {
    throw BadNumCast(exception.what());
  }
}

}  // namespace gunit
