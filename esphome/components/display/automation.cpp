#include "automation.h"
#include "esphome/core/log.h"

namespace esphome {
namespace display {

bool match_interval(uint32_t min_length, uint32_t max_length, uint32_t length) {
  if (max_length == 0) {
    return length >= min_length;
  } else {
    return length >= min_length && length <= max_length;
  }
}

}  // namespace display
}  // namespace esphome
