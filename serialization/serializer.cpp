// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#include "serializer.h"
#include "serializable.h"

#include <fstream>

namespace cider {
namespace serialization {

Serializer::Serializer() {
  m_buffer = std::make_unique<std::uint8_t[]>(4096);
  m_base = m_buffer.get();
  m_pointer = m_buffer.get();
  m_size = 4096;
}

Serializer::Serializer(size_t size) {
  m_buffer = std::make_unique<std::uint8_t[]>(size);
  m_base = m_buffer.get();
  m_pointer = m_buffer.get();
  m_size = size;
}

void Serializer::save(const std::string& filePath) {
  if (!m_buffer || m_size == 0) {
    throw std::runtime_error("No data to write.");
  }

  std::ofstream outFile(filePath, std::ios::binary);
  if (!outFile) {
    throw std::runtime_error("Failed to open file for writing: " + filePath);
  }

  const auto used = Used();
  outFile.write(reinterpret_cast<const char*>(m_base), used);
  if (!outFile) {
    throw std::runtime_error("Failed to write data to file: " + filePath);
  }
}

void Serializer::Resize(size_t size) {
  const auto used = Used();
  if (size > m_size) {
    auto prevData = std::move(m_buffer);
    auto prevSize = m_size;
    m_buffer = std::make_unique<std::uint8_t[]>(size);
    m_size = size;
    memcpy(m_buffer.get(), prevData.get(), prevSize);
  } else {
    m_size = size;
  }

  m_base = m_buffer.get();
  m_pointer = m_buffer.get() + used;
  m_size = size;
}

size_t Serializer::Used() const {
  return m_pointer - m_base;
}

bool Serializer::AddFixed(const void* data, size_t size) {
  if (Used() + size > m_size) {
    Resize(Used() + size);
  }

  memcpy(m_pointer, data, size);
  m_pointer += size;
  return true;
}

bool Serializer::operator<<(double data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(float data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(void* data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(char data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(unsigned char data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(short data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(unsigned short data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(int data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(unsigned int data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(long data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(unsigned long data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(long long data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(unsigned long long data) {
  return AddFixed(&data, sizeof(data));
}

bool Serializer::operator<<(bool data_) {
  std::uint8_t data = data_ ? 1 : 0;
  return operator<<(data);
}

bool Serializer::AddSize(size_t size_) {
  auto size = static_cast<t_size>(size_);
  return AddFixed(&size, sizeof(size));
}

bool Serializer::AddSized(const void* data, size_t size) {
  bool result = AddSize(size);
  return result & AddFixed(data, size);
}

bool Serializer::operator<<(const ISerializable& serializable) {
  return serializable.serialize(*this);
}

}  // namespace serialization
}  // namespace cider
