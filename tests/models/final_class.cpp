// Copyright (C) 2022-2024 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "final_class.h"

namespace cider {
namespace models {

FinalClass::FinalClass() : _number(0), _condition(false) {}
FinalClass::FinalClass(int number, bool condition)
    : _number(number), _condition(condition) {}

FinalClass::FinalClass(const FinalClass& other)
    : _number(other._number), _condition(other._condition) {}
FinalClass::FinalClass(FinalClass&& other)
    : _number(other._number), _condition(other._condition) {}

void FinalClass::someMethod(int newNumber) {
  _number = newNumber;
}

FinalClass& FinalClass::operator=(const FinalClass& other) {
  _number = other._number;
  _condition = other._condition;
  return *this;
}

FinalClass operator+(const FinalClass& left, const FinalClass& right) {
  FinalClass cl;
  cl._number = left._number + right._number;
  return cl;
}

FinalClass::operator bool() const {
  return _condition;
}

FinalClass& FinalClass::operator=(FinalClass&& other) {
  _number = other._number;
  _condition = other._condition;
  return *this;
}

bool FinalClass::operator==(const FinalClass& that) const {
  return _condition == that._condition && _number == that._number;
}
bool FinalClass::operator!=(const FinalClass& that) const {
  return (*this) != that;
}

FinalClass& FinalClass::operator+=(const unsigned int that) {
  _number += that;
  return *this;
}

FinalClass& FinalClass::operator+=(const int& that) {
  _number += that;
  return *this;
}

FinalClass& FinalClass::operator=(const unsigned int that) {
  _number = that;
  return *this;
}

FinalClass& FinalClass::operator=(const int& that) {
  _number = that;
  return *this;
}

FinalClass& FinalClass::operator=(const int&& that) {
  _number = that;
  return *this;
}

FinalClass function_make_class_construct_obj() {
  return FinalClass{4351, true};
}

FinalClass* function_make_class_construct_obj_ptr() {
  return new FinalClass{411, true};
}

FinalClass function_test_class_construct(const FinalClass& arg) {
  return arg;
}

FinalClass* function_test_class_construct_same_pointer_return(FinalClass* arg) {
  return arg;
}

FinalClass* function_test_class_construct(FinalClass* arg) {
  return new FinalClass{*arg};
}

}  // namespace models
}  // namespace cider
