// Copyright (C) 2022-2025 Hulevych Mykhailo
// SPDX-License-Identifier: MIT

#pragma once

#include "serialization/deserializer.h"
#include "serialization/serializer.h"

namespace cider {
namespace serialization {

class ISerializable {
 public:
  virtual ~ISerializable() = default;

  virtual bool serialize(Serializer& serializer) const = 0;
  virtual bool deserialize(const Deserializer& deserializer) = 0;
};

struct SerializableTag {};

}  // namespace serialization
}  // namespace cider
