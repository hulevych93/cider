#pragma once

#include <cppast/cppast_fwd.hpp>

#include "metadata.h"

namespace gunit {
namespace tool {

enum OperatorType { copyAssignment, moveAssignment, Equals, NotEquals };

std::optional<OperatorType> isOperator(const cppast::cpp_member_function& e);

bool isMoveContructor(const cppast::cpp_constructor& e);

bool hasReturnValue(const cppast::cpp_function& e);

bool hasReturnValue(const cppast::cpp_member_function& e);

bool isUserDefined(const cppast::cpp_type& type, std::string& name);

bool isUserData(const cppast::cpp_type& type, const MetadataStorage& metadata);

bool isAggregate(const cppast::cpp_type& type, const MetadataStorage& metadata);

bool hasImpl(const cppast::cpp_type& type,
             const MetadataStorage& metadata,
             std::string& name);

}  // namespace tool
}  // namespace gunit
