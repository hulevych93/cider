// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "deserializer.h"
#include "serializable.h"

#include <cstring>
#include <fstream>
#include <stdexcept>

namespace cider {
namespace serialization {

Deserializer::Deserializer(const void* data, size_t size)
    : m_base(static_cast<const std::uint8_t* const>(data)),
      m_data(const_cast<std::uint8_t*>(static_cast<const std::uint8_t*>(data))),
      m_size(size) {
  if (!m_data || !m_size) {
    throw std::invalid_argument("invalid_argument");
  }
}

Deserializer::Deserializer(const std::string& filePath)
    : m_base(nullptr), m_data(nullptr), m_size(0) {
  std::ifstream file(filePath, std::ios::binary | std::ios::ate);
  if (!file) {
    throw std::runtime_error("Failed to open file: " + filePath);
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  auto* buffer = new std::uint8_t[size];
  if (!file.read(reinterpret_cast<char*>(buffer), size)) {
    delete[] buffer;
    throw std::runtime_error("Failed to read file: " + filePath);
  }

  m_data = buffer;
  *const_cast<std::uint8_t**>(&m_base) = buffer;
  *const_cast<size_t*>(&m_size) = static_cast<size_t>(size);
}

size_t Deserializer::Used() const {
  return m_data - m_base;
}

bool Deserializer::PeekFixed(void* peek, size_t size) const {
  if (Used() + size <= m_size) {
    memcpy(peek, m_data, size);
    return true;
  } else {
    throw std::out_of_range("out_of_range");
  }
}

bool Deserializer::GetFixed(void* get, size_t size) const {
  if (Used() + size <= m_size) {
    memcpy(get, m_data, size);
    m_data += size;
    return true;
  } else {
    throw std::out_of_range("out_of_range");
  }
}

bool Deserializer::operator>>(void*& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(double& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(float& data) const {
  return GetFixed(&data, sizeof(data));
}
bool Deserializer::operator>>(char& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(unsigned char& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(short& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(unsigned short& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(int& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(unsigned int& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(long& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(unsigned long& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(long long& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(unsigned long long& data) const {
  return GetFixed(&data, sizeof(data));
}

bool Deserializer::operator>>(bool& data_) const {
  std::uint8_t data;
  operator>>(data);
  data_ = data ? true : false;
  return true;
}

bool Deserializer::GetSize(size_t& size) const {
  if (Used() + sizeof(t_size) <= m_size) {
    auto sizeRaw = *reinterpret_cast<const t_size*>(m_data);
    size = static_cast<size_t>(sizeRaw);
    m_data += sizeof(sizeRaw);
    return true;
  } else {
    throw std::out_of_range("out_of_range");
  }
}

bool Deserializer::GetSized(t_getter getter) const {
  size_t size;
  GetSize(size);

  if (Used() + size <= m_size) {
    getter(m_data, size);
    m_data += size;
    return true;
  } else {
    throw std::out_of_range("out_of_range");
  }
}

bool Deserializer::operator>>(ISerializable& serializable) const {
  serializable.deserialize(const_cast<Deserializer&>(*this));
  return true;
}

}  // namespace serialization
}  // namespace cider
