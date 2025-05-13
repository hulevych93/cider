// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "serialization/common.h"

namespace cider {
namespace serialization {

class ISerializable;
struct SerializableTag;

class Serializer final {
 private:
  using t_size = std::uint32_t;

 public:
  Serializer();
  Serializer(size_t size);

  void save(const std::string& filePath);

  std::uint8_t* getData() const { return m_buffer.get(); }
  size_t getSize() const { return Used(); }

 public:
  bool operator<<(double data);
  bool operator<<(float data);
  bool operator<<(char data);
  bool operator<<(unsigned char data);
  bool operator<<(short data);
  bool operator<<(unsigned short data);
  bool operator<<(int data);
  bool operator<<(unsigned int data);
  bool operator<<(long data);
  bool operator<<(unsigned long data);
  bool operator<<(long long data);
  bool operator<<(unsigned long long data);
  bool operator<<(bool data);
  bool operator<<(const ISerializable& serializable);
  bool operator<<(void* data);

  template <class T>
  typename std::enable_if<std::is_enum<T>::value, bool>::type operator<<(
      const T& data);

  template <class T>
  typename std::enable_if<std::is_base_of_v<SerializableTag, T>, bool>::type
  operator<<(const T& data);

  template <class _Elem, class _Traits, class _Alloc>
  bool operator<<(const std::basic_string<_Elem, _Traits, _Alloc>& string);

  template <class T>
  bool operator<<(const std::shared_ptr<T>& object);

  template <class T>
  bool operator<<(const std::unique_ptr<T>& object);

  template <class T>
  bool operator<<(const std::optional<T>& object);

  template <class... T>
  bool operator<<(const std::variant<T...>& object);

  template <class T>
  bool operator()(const T& operand);

  template <class T>
  bool operator<<(const std::vector<T>& container);

  template <class T>
  bool operator<<(const std::deque<T>& container);

  template <class T>
  bool operator<<(const std::list<T>& container);

  template <class T, class P, class A>
  bool operator<<(const std::set<T, P, A>& container);

  template <class T>
  bool operator<<(const std::unordered_set<T>& container);

  template <class K, class V>
  bool operator<<(const std::map<K, V>& container);

  template <class K, class V, class B, class N>
  bool operator<<(const std::unordered_map<K, V, B, N>& container);

  template <class K, class V>
  bool operator<<(const std::pair<K, V>& pair);

 private:
  bool AddFixed(const void* data, size_t size);
  bool AddSized(const void* data, size_t size);
  bool AddSize(size_t size);

  size_t Used() const;
  void Resize(size_t size);

  template <class T>
  bool AddContainer(const T& container);

 private:
  const std::uint8_t* m_base;
  std::uint8_t* m_pointer;
  std::unique_ptr<std::uint8_t[]> m_buffer;
  size_t m_size;
};

template <class T>
typename std::enable_if<std::is_enum<T>::value, bool>::type
Serializer::operator<<(const T& data) {
  return AddFixed(&data, sizeof(data));
}

template <class T>
typename std::enable_if<std::is_base_of_v<SerializableTag, T>, bool>::type
Serializer::operator<<(const T& data) {
  return serialize(data, *this);
}

template <class _Elem, class _Traits, class _Alloc>
bool Serializer::operator<<(
    const std::basic_string<_Elem, _Traits, _Alloc>& string) {
  return AddSized(string.data(), string.size() * sizeof(_Elem));
}

template <class T>
bool Serializer::operator<<(const std::shared_ptr<T>& object) {
  bool result = operator<<(object != nullptr ? static_cast<short>(1)
                                             : static_cast<short>(0));
  if (object)
    result &= operator<<(*object);
  return result;
}

template <class T>
bool Serializer::operator<<(const std::unique_ptr<T>& object) {
  bool result = operator<<(object != nullptr ? static_cast<short>(1)
                                             : static_cast<short>(0));
  if (object)
    result &= operator<<(*object);
  return result;
}

template <class T>
bool Serializer::operator<<(const std::optional<T>& object) {
  bool result = operator<<(object ? static_cast<short>(1)
                                  : static_cast<short>(0));
  if (object)
    result &= operator<<(object.value());
  return result;
}

template <class... T>
bool Serializer::operator<<(const std::variant<T...>& object) {
  if (operator<<((short)object.index())) {
    return std::visit(*this, object);
  }
  return false;
}

template <class T>
bool Serializer::operator()(const T& operand) {
  return operator<<(operand);
}

template <class T>
bool Serializer::operator<<(const std::vector<T>& container) {
  return AddContainer(container);
}

template <class T>
bool Serializer::operator<<(const std::deque<T>& container) {
  return AddContainer(container);
}

template <class T>
bool Serializer::operator<<(const std::list<T>& container) {
  return AddContainer(container);
}

template <class T, class P, class A>
bool Serializer::operator<<(const std::set<T, P, A>& container) {
  return AddContainer(container);
}

template <class T>
bool Serializer::operator<<(const std::unordered_set<T>& container) {
  return AddContainer(container);
}

template <class K, class V>
bool Serializer::operator<<(const std::map<K, V>& container) {
  return AddContainer(container);
}

template <class K, class V, class B, class N>
bool Serializer::operator<<(const std::unordered_map<K, V, B, N>& container) {
  return AddContainer(container);
}

template <class K, class V>
bool Serializer::operator<<(const std::pair<K, V>& pair) {
  bool result = operator<<(pair.first);
  result &= operator<<(pair.second);
  return result;
}

template <class T>
bool Serializer::AddContainer(const T& container) {
  bool result = AddSize(container.size());
  for (const auto& value : container) {
    result &= operator<<(value);
  }
  return result;
}

}  // namespace serialization
}  // namespace cider
