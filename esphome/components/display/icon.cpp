#include "icon.h"
#include "image.h"
#include "display_buffer.h"

namespace esphome {
namespace display {

void Icon::lazy_init(DisplayBuffer &display) {
  Position pos{get_pos()};
  this->x2_ = pos.width;
  this->y2_ = pos.height;
}

void Icon::draw_component(DisplayBuffer &display) {
  Position pos{get_pos()};
  display.filled_rectangle(pos.x, pos.y, pos.width+1, pos.height+1, this->background_.to_16bit());
  display.image_section(pos.x - this->x1_, pos.y - this->y1_, this->x1_, this->y1_, this->x2_, this->y2_, this->image_);
}

void Icon::set_image(Image* image) {
  this->image_ = image; this->refresh();
}

void Icon::crop(int x1, int y1, int x2, int y2) {
  this->x1_ = x1;
  this->y1_ = y1;
  this->x2_ = x2;
  this->y2_ = y2;
  this->refresh();
}

}  // namespace display
}  // namespace esphome
