#pragma once

namespace gunit {
namespace models {

class NoConstructors final {
 public:
  void doo();

 private:
  int m_data = 0;
};

class OnlyDestructor final {
 public:
  ~OnlyDestructor();

  void doo();

 private:
  int m_data = 0;
};

}  // namespace models
}  // namespace gunit
