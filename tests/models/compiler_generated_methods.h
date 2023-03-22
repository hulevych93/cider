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

  // no copy assignment operator will be generated
  // no move assignment operator will be generated

  void doo();

 private:
  int m_data = 0;
};

class DefaultAndCopyConstructor final {
 public:
  DefaultAndCopyConstructor();
  DefaultAndCopyConstructor(const DefaultAndCopyConstructor& that);
  // no move con-tor will be generated
  // no move assignment operator will be generated

  void doo();

 private:
  int m_data = 0;
};

class OnlyCopyOperator final {
 public:
  OnlyCopyOperator& operator=(const OnlyCopyOperator& that);

  // no move assignment operator will be generated

  void doo();

 private:
  int m_data = 0;
};

class OnlyMoveOperator final {
 public:
  OnlyMoveOperator& operator=(OnlyMoveOperator&& that);
  // no copy con-tor will be generated

  // no copy assignment operator will be generated

  void doo();

 private:
  int m_data = 0;
};

}  // namespace models
}  // namespace gunit
