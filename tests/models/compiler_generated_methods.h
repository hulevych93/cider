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

class UserConstructor final {
 public:
  explicit UserConstructor(int data);

  void doo();

 private:
  int m_data = 0;
};

class DefaultAndMoveConstructor final {
 public:
  DefaultAndMoveConstructor();
  DefaultAndMoveConstructor(DefaultAndMoveConstructor&& that);
  // no copy con-tor will be generated

  void doo();

 private:
  int m_data = 0;
};

}  // namespace models
}  // namespace gunit
