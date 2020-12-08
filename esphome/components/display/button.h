#pragma once

#include "drawable.h"
#include <string>

namespace esphome {
namespace display {

class DisplayBuffer;
class Font;
class Image;

class Button : public Drawable {
 public:
  void lazy_init(DisplayBuffer &display) override;
  void draw_component(DisplayBuffer &display) override;
  void set_text(const char* text) { this->text_ = text; }
  void set_font(Font *font) { this->font_ = font; }
  void set_image(Image* image) { this->image_ = image; }
  void set_image_pressed(Image* image) { this->image_pressed_ = image; }

 private:
  Image* image_{nullptr};
  Image* image_pressed_{nullptr};
  std::string text_;
  Font* font_{nullptr};
  bool pressed_{false};
};

}  // namespace display
}  // namespace esphome
