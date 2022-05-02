#ifndef NPSYSTEM_CONSTANTS_
#define NPSYSTEM_CONSTANTS_

#include <nprpc/flat.hpp>
#include <nprpc/nprpc.hpp>

namespace npsysc { 
enum class Quality : uint8_t {
  VQ_BAD = 0x00,
  VQ_GOOD = 0x01
};
enum class Boolean : uint8_t {
  True = 0xff,
  False = 0x00
};
} // namespace npsysc

namespace npsystem_constants::helper {
} // namespace npsystem_constants::helper

#endif