#include "user_data.h"

namespace gunit {
namespace recorder {

bool operator==(const UserDataParamPtr& left, const UserDataParamPtr& right) {
  return (*left).equals(*right);
}

bool operator!=(const UserDataParamPtr& left, const UserDataParamPtr& right) {
  return !operator==(left, right);
}

}  // namespace recorder
}  // namespace gunit
