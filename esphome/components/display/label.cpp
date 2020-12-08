#include "label.h"
#include "display_buffer.h"
#include "font.h"

namespace esphome {
namespace display {

void Label::draw_component(DisplayBuffer &display) {
  Position pos{get_pos()};
  display.filled_rectangle(pos.x, pos.y, pos.width, pos.height, this->background_.to_16bit());
  display.print(pos.x, pos.y, this->font_, this->color_.to_16bit(), this->text_.c_str());
}

void Label::set_text(const char* text) {
  this->text_ = text; this->refresh();
}

void Label::set_font(Font *font) {
  this->font_ = font; this->refresh();
}

void Label::set_color(ColorRGB color) {
  this->color_ = color; this->refresh();
}

}  // namespace display
}  // namespace esphome
