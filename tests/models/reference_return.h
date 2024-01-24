#pragma once

namespace cider {
namespace models {

struct RefStruct final {
  RefStruct& self();
  RefStruct copy();

  int inc();

 private:
  int field = 1;
};

}  // namespace models
}  // namespace cider
