#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include <functional>
#include <vector>

namespace esphome {
namespace touchscreen {

enum TouchscreenRotation {
  TOUCHSCREEN_ROTATION_0_DEGREES = 0,
  TOUCHSCREEN_ROTATION_90_DEGREES = 90,
  TOUCHSCREEN_ROTATION_180_DEGREES = 180,
  TOUCHSCREEN_ROTATION_270_DEGREES = 270,
};

#define LOG_TOUCHSCREEN(prefix, type, obj) \
  if (obj != nullptr) { \
    ESP_LOGCONFIG(TAG, prefix type); \
    ESP_LOGCONFIG(TAG, "%s  Rotations: %d °", prefix, obj->rotation_); \
  }


enum class TouchPhase {
  START,
  CONTINUE,
  END,
};

struct TouchEvent {
  TouchPhase p;
  uint32_t x;
  uint32_t y;
  uint32_t z;
};

class Touchscreen {
 public:
  virtual ~Touchscreen(){}
  virtual void add_touch_handler(std::function<void(TouchEvent)> listener) { listeners_.push_back(listener); }
  void set_rotation(TouchscreenRotation rotation);
  void set_pixel_x(uint32_t pixel_x) { this->pixel_x_ = pixel_x; }
  void set_pixel_y(uint32_t pixel_y) { this->pixel_y_ = pixel_y; }

 protected:
  TouchEvent rotate_event(const TouchEvent event);
  void notify_listeners(TouchEvent event);
  std::vector<std::function<void(TouchEvent)>> listeners_;
  TouchscreenRotation rotation_{TOUCHSCREEN_ROTATION_0_DEGREES};
  uint32_t pixel_x_{320};
  uint32_t pixel_y_{240};
};


}  // namespace touchscreen
}  // namespace esphome
