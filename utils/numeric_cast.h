#pragma once

#include <exception>

namespace gunit {
class BadNumCast final : public std::exception {
 public:
  const char* what() const _NOEXCEPT override { return "bad num_cast"; }
};

template <typename To,
          typename From,
          std::enable_if_t<std::is_same<To, From>::value>* = nullptr>
To numCast(From from) {
  return from;
}

template <typename To,
          typename From,
          std::enable_if_t<!std::is_same<To, From>::value>* = nullptr>
To numCast(From from) {
  // TODO: implement safe cast;
  return static_cast<To>(from);
}

}  // namespace gunit
