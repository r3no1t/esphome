#pragma once

#include "drawable.h"
#include <string>

namespace esphome {
namespace display {

class DisplayBuffer;
class Font;

class Label : public Drawable {
 public:
  void draw_component(DisplayBuffer &display) override;
  void set_text(const char* text);
  void set_font(Font *font);
  void set_color(ColorRGB color);

 private:
  std::string text_;
  Font* font_{nullptr};
  ColorRGB color_{0,0,0};
};

}  // namespace display
}  // namespace esphome
