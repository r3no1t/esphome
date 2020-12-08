#include "button.h"
#include "display_buffer.h"
#include "font.h"
#include "image.h"

namespace esphome {
namespace display {

void Button::lazy_init(DisplayBuffer &display) {
  auto* touch = display.get_touchscreen();
  if (touch != nullptr) touch->add_touch_handler([&display,this](esphome::touchscreen::TouchEvent e){
    if (!this->is_visible()) return;
    Position pos{get_pos()};

    if (e.x > pos.x && e.x < (pos.x + pos.width) && e.y > pos.y && e.y < (pos.y + pos.height)) {
      ESP_LOGD("display", "button press");
      if (e.p == esphome::touchscreen::TouchPhase::START) {
        this->pressed_ = true;
        this->refresh();
        ESP_LOGD("display", "button callback");
        this->state_callback_.call(this->pressed_);
      }
      else if (this->pressed_ && e.p == esphome::touchscreen::TouchPhase::END) {
        this->pressed_ = false;
        this->refresh();
        this->state_callback_.call(this->pressed_);
      }
    }
    else {
      this->pressed_ = false;
      this->refresh();
    }
  });
}

void Button::draw_component(DisplayBuffer &display) {
  Position pos{get_pos()};
  int text_width = 0;
  int text_height = 0;

  if (this->text_.size() > 0 && this->font_ != nullptr) {
    int x,y;
    display.get_text_bounds(0, 0, this->text_.c_str(), this->font_, TextAlign::TOP_LEFT, &x, &y, &text_width, &text_height);
  }
  if ((this->image_ != nullptr) && (this->image_pressed_ != nullptr)) {
    Image* image = (this->pressed_) ? this->image_pressed_ : this->image_;
    display.image(pos.x + (pos.width - image->get_width() - text_width) / ((this->text_.size() > 0) ? 3 : 2),
                  pos.y + (pos.height - image->get_height()) / 2, image);
    if (this->text_.size() > 0 && this->font_ != nullptr) {
      display.print(pos.x + this->image_->get_width() + 2 * (pos.width - this->image_->get_width() - text_width) / 3,
                    pos.y + (pos.height - text_height) / 2, this->font_, this->text_.c_str());
    }
  }
  else if (this->text_.size() > 0 && this->font_ != nullptr) {
    display.print(pos.x + (pos.width - text_width) / 2, pos.y + (pos.height - text_height) / 2, this->font_, this->text_.c_str());
  }
}

}  // namespace display
}  // namespace esphome
