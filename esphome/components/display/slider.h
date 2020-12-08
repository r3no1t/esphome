#pragma once

#include "drawable.h"

namespace esphome {
namespace display {

class DisplayBuffer;
class Image;

class Slider : public Drawable {
 public:
  void lazy_init(DisplayBuffer &display) override;
  void draw_component(DisplayBuffer &display) override;
  void set_image_button(Image* image);
  void set_image_button_pressed(Image* image);
  void set_image_background(Image* image);
  void set_value(int value);
  int get_value();

private:
  Position get_button_pos();

  Image* image_button_{nullptr};
  Image* image_button_pressed_{nullptr};
  Image* image_background_{nullptr};
  int value_{0};
  int last_sent_value_{-1};
  int y_start_{0};
  bool pressed_{false};
};

}  // namespace display
}  // namespace esphome
