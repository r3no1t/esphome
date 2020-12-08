#include "slider.h"
#include "display_buffer.h"
#include "image.h"

namespace esphome {
namespace display {

void Slider::lazy_init(DisplayBuffer &display) {
  auto* touch = display.get_touchscreen();
  if (touch != nullptr) touch->add_touch_handler([&display,this](esphome::touchscreen::TouchEvent e){
    if (!this->is_visible()) return;
    Position pos{get_pos()};
    Position button_pos{get_button_pos()};
    if (e.x > button_pos.x && e.x < (button_pos.x + button_pos.width) &&
        e.y > button_pos.y && e.y < (button_pos.y + button_pos.height)) {
      if (e.p == esphome::touchscreen::TouchPhase::START) {
        this->y_start_ = e.y;
        this->pressed_ = true;
      }
    }
    if (this->pressed_) {
      this->value_ = clamp(((this->y_start_ - (int)e.y) * 100) / (pos.height - this->image_button_->get_height()) + this->value_, 0, 100);
      ESP_LOGD("display", "value:%d", this->value_);
      this->y_start_ = e.y;
      if ((e.p == esphome::touchscreen::TouchPhase::END) || (e.x < (button_pos.x - 20) || e.x > (button_pos.x + button_pos.width + 20))) this->pressed_ = false;
      if (this->last_sent_value_ != this->value_) {
        this->last_sent_value_ = this->value_;
        this->state_callback_.call(this->pressed_);
      }
    }
    this->refresh();
  });
}

void Slider::draw_component(DisplayBuffer &display) {
  Position pos{get_pos()};
  if (this->image_button_ != nullptr && this->image_background_ != nullptr) {
    display.image(pos.x, pos.y, this->image_background_);
    Image* image = this->pressed_ ? this->image_button_pressed_ : this->image_button_;
    Position button_pos{get_button_pos()};
    display.image(button_pos.x, button_pos.y, image);
  }
}

void Slider::set_image_button(Image* image) {
  this->image_button_ = image;
}

void Slider::set_image_button_pressed(Image* image) {
  this->image_button_pressed_ = image;
}

void Slider::set_image_background(Image* image) {
  this->image_background_ = image;
}

void Slider::set_value(int value) {
  this->value_ = value;
}

int Slider::get_value() {
  return this->value_;
}

Position Slider::get_button_pos() {
  Position pos{get_pos()};
  Position button_pos;
  button_pos.x = pos.x + (pos.width - this->image_button_->get_width()) / 2;
  button_pos.y = pos.y + (pos.height - this->image_button_->get_height()) * (100 - value_) / 100;
  button_pos.width = this->image_button_->get_width();
  button_pos.height = this->image_button_->get_height();
  return button_pos;
}

}  // namespace display
}  // namespace esphome
