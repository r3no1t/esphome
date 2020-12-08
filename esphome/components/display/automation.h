#pragma once

#include "drawable.h"
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/display/display_buffer.h"

namespace esphome {
namespace display {

class PressTrigger : public Trigger<> {
 public:
  explicit PressTrigger(Drawable *drawable) {
    drawable->add_on_press_callback([this](bool state) {
      if (state)
        this->trigger();
    });
  }
};

class ReleaseTrigger : public Trigger<> {
 public:
  explicit ReleaseTrigger(Drawable *drawable) {
    drawable->add_on_press_callback([this](bool state) {
      if (!state)
        this->trigger();
    });
  }
};

bool match_interval(uint32_t min_length, uint32_t max_length, uint32_t length);

class ClickTrigger : public Trigger<> {
 public:
  explicit ClickTrigger(Drawable *drawable, uint32_t min_length, uint32_t max_length)
      : min_length_(min_length), max_length_(max_length) {
    drawable->add_on_press_callback([this](bool state) {
      if (state) {
        this->start_time_ = millis();
      } else {
        const uint32_t length = millis() - this->start_time_;
        if (match_interval(this->min_length_, this->max_length_, length))
          this->trigger();
      }
    });
  }

 protected:
  uint32_t start_time_{0};  /// The millis() time when the click started.
  uint32_t min_length_;     /// Minimum length of click. 0 means no minimum.
  uint32_t max_length_;     /// Maximum length of click. 0 means no maximum.
};

}  // namespace display
}  // namespace esphome
