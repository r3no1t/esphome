#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "touchscreen.h"

namespace esphome {
namespace touchscreen {

static const char *TAG = "touchscreen";

void Touchscreen::set_rotation(TouchscreenRotation rotation) {
  this->rotation_ = rotation;
}

void Touchscreen::notify_listeners(TouchEvent event) {
  event = rotate_event(event);
  ESP_LOGD(TAG, "x:%d y:%d z:%d", event.x, event.y, event.z);
  for (auto& listener : this->listeners_) {
    listener(event);
  }
}

TouchEvent Touchscreen::rotate_event(const TouchEvent event) {
  TouchEvent e{event};
  switch(this->rotation_) {
  case TOUCHSCREEN_ROTATION_90_DEGREES:
    e.x = (this->pixel_x_ * event.y) / this->pixel_y_;
    e.y = this->pixel_y_ - (this->pixel_y_ * event.x) / this->pixel_x_;
    break;
  case TOUCHSCREEN_ROTATION_180_DEGREES:
    e.x = this->pixel_x_ - event.x;
    e.y = this->pixel_y_ - event.y;
    break;
  case TOUCHSCREEN_ROTATION_270_DEGREES:
    e.x = this->pixel_x_ - (this->pixel_x_ * event.y) / this->pixel_y_;
    e.y = (this->pixel_y_ * event.x) / this->pixel_x_;
    break;
  default:
    break;
  }
  return e;
}

}  // namespace touchscreen
}  // namespace esphome
