// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "serialization/common.h"

namespace cider {
namespace serialization {

class ISerializable;
struct SerializableTag;

class Deserializer final {
 private:
  using t_size = std::uint32_t;
  using t_getter = std::function<void(const std::uint8_t* data, size_t size)>;

 public:
  Deserializer(const std::string& filePath);
  Deserializer(const void* data, size_t size);

  bool operator>>(void*& data) const;

  bool operator>>(double& data) const;
  bool operator>>(float& data) const;

  bool operator>>(char& data) const;
  bool operator>>(unsigned char& data) const;

  bool operator>>(short& data) const;
  bool operator>>(unsigned short& data) const;
  bool operator>>(int& data) const;
  bool operator>>(unsigned int& data) const;
  bool operator>>(long& data) const;
  bool operator>>(unsigned long& data) const;
  bool operator>>(long long& data) const;
  bool operator>>(unsigned long long& data) const;

  bool operator>>(bool& data) const;
  bool operator>>(ISerializable& serializable) const;

  template <class T>
  typename std::enable_if<std::is_enum<T>::value, bool>::type operator>>(
      T& data) const;

  template <class T>
  typename std::enable_if<std::is_base_of_v<SerializableTag, T>, bool>::type
  operator>>(T& data) const;

  template <class _Elem, class _Traits, class _Alloc>
  bool operator>>(std::basic_string<_Elem, _Traits, _Alloc>& string) const;

  template <class T>
  bool operator>>(std::unique_ptr<T>& object) const;

  template <class T>
  bool operator>>(std::shared_ptr<T>& value) const;

  template <class T>
  bool operator>>(std::optional<T>& object) const;

  template <class... T>
  bool operator>>(std::variant<T...>& object) const;

  template <class T>
  bool operator()(T& operand) const;

  template <class T>
  bool operator>>(std::vector<T>& container) const;

  template <class T>
  bool operator>>(std::deque<T>& container) const;

  template <class T>
  bool operator>>(std::list<T>& container) const;

  template <class T>
  bool operator>>(std::set<T>& container) const;

  template <class T>
  bool operator>>(std::unordered_set<T>& container) const;

  template <class K, class V>
  bool operator>>(std::map<K, V>& container) const;

  template <class T, class P, class A>
  bool operator>>(std::set<T, P, A>& container) const;

  template <class K, class V, class B, class N>
  bool operator>>(std::unordered_map<K, V, B, N>& container) const;

  template <class K, class V>
  bool operator>>(std::pair<K, V>& pair) const;

 private:
  bool PeekFixed(void* peek, size_t size) const;
  bool GetFixed(void* get, size_t size) const;
  bool GetSized(t_getter getter) const;
  bool GetSize(size_t& size) const;
  size_t Used() const;

 private:
  template <class T>
  bool GetContainerSingle(T& container) const;

  template <class T>
  bool GetContainerSingleSet(T& container) const;

  template <class T>
  bool GetContainerPaired(T& container) const;

  template <class T>
  bool GetShared(std::shared_ptr<T>& value, std::true_type) const;

  template <class T>
  bool GetShared(std::shared_ptr<T>& value, std::false_type) const;

 private:
  const std::uint8_t* const m_base;
  mutable std::uint8_t* m_data;
  const size_t m_size;
};

template <class T>
typename std::enable_if<std::is_enum<T>::value, bool>::type
Deserializer::operator>>(T& data) const {
  return GetFixed(&data, sizeof(data));
}

template <class T>
typename std::enable_if<std::is_base_of_v<SerializableTag, T>, bool>::type
Deserializer::operator>>(T& data) const {
  return deserialize(data, *this);
}

template <class _Elem, class _Traits, class _Alloc>
bool Deserializer::operator>>(
    std::basic_string<_Elem, _Traits, _Alloc>& string) const {
  return GetSized([&](const std::uint8_t* data, size_t size) {
    string.assign(reinterpret_cast<const _Elem*>(data), size / sizeof(_Elem));
  });
}

template <class T>
bool Deserializer::operator>>(std::unique_ptr<T>& object) const {
  short res = {0};
  bool result = operator>>(res);
  if (res > 0) {
    object = std::make_unique<T>();
    result &= operator>>(*object);
  }
  return result;
}

template <class T>
bool Deserializer::operator>>(std::shared_ptr<T>& value) const {
  short res = {0};
  bool result = operator>>(res);
  if (res > 0) {
    result &= GetShared(
        value, std::integral_constant < bool,
        std::is_final<T>::value || std::is_fundamental<T>::value > {});
  }
  return result;
}

template <class T>
bool Deserializer::operator>>(std::optional<T>& object) const {
  short res = {0};
  bool result = operator>>(res);
  if (res > 0) {
    T value;
    result &= operator>>(value);
    object = value;
  }
  return result;
}

template <class... T>
bool Deserializer::operator>>(std::variant<T...>& object) const {
  short which{0};
  if (operator>>(which)) {
    makeVariant(which, object);
    return std::visit(*this, object);
  }
  return false;
}

template <class T>
bool Deserializer::operator()(T& operand) const {
  return operator>>(operand);
}

template <class T>
bool Deserializer::operator>>(std::vector<T>& container) const {
  return GetContainerSingle(container);
}

template <class T>
bool Deserializer::operator>>(std::deque<T>& container) const {
  return GetContainerSingle(container);
}

template <class T>
bool Deserializer::operator>>(std::list<T>& container) const {
  return GetContainerSingle(container);
}

template <class T>
bool Deserializer::operator>>(std::set<T>& container) const {
  return GetContainerSingleSet(container);
}

template <class T>
bool Deserializer::operator>>(std::unordered_set<T>& container) const {
  return GetContainerSingleSet(container);
}

template <class K, class V>
bool Deserializer::operator>>(std::map<K, V>& container) const {
  return GetContainerPaired(container);
}

template <class T, class P, class A>
bool Deserializer::operator>>(std::set<T, P, A>& container) const {
  return GetContainerSingleSet(container);
}

template <class K, class V, class B, class N>
bool Deserializer::operator>>(std::unordered_map<K, V, B, N>& container) const {
  return GetContainerPaired(container);
}

template <class K, class V>
bool Deserializer::operator>>(std::pair<K, V>& pair) const {
  bool result = operator>>(pair.first);
  result &= operator>>(pair.second);
  return result;
}

template <class T>
bool Deserializer::GetContainerSingle(T& container) const {
  size_t size;
  bool result = GetSize(size);

  for (size_t i = 0; i < size; i++) {
    typename T::value_type value;
    operator>>(value);
    container.emplace(container.end(), value);
  }
  return result;
}

template <class T>
bool Deserializer::GetContainerSingleSet(T& container) const {
  size_t size;
  bool result = GetSize(size);

  for (size_t i = 0; i < size; i++) {
    typename T::value_type value;
    operator>>(value);
    container.emplace(value);
  }
  return result;
}

template <class T>
bool Deserializer::GetContainerPaired(T& container) const {
  size_t size;
  bool result = GetSize(size);

  for (size_t i = 0; i < size; i++) {
    typename T::key_type key;
    typename T::mapped_type mapped;
    operator>>(key);
    operator>>(mapped);
    container.emplace(key, mapped);
  }
  return result;
}

template <class T>
bool Deserializer::GetShared(std::shared_ptr<T>& value, std::true_type) const {
  value = std::make_shared<T>();
  return operator>>(*value);
}

template <class T>
bool Deserializer::GetShared(std::shared_ptr<T>& value, std::false_type) const {
  value = T::Create(*this);
  return operator>>(*value);
}

}  // namespace serialization
}  // namespace cider
