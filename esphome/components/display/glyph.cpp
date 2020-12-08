#include "glyph.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace display {

Glyph::Glyph(const char *a_char, const uint8_t *data_start, uint32_t offset, int offset_x, int offset_y, int width,
             int height)
    : char_(a_char),
      data_(data_start + offset),
      offset_x_(offset_x),
      offset_y_(offset_y),
      width_(width),
      height_(height) {
}

bool Glyph::get_pixel(int x, int y) const {
  const int x_data = x - this->offset_x_;
  const int y_data = y - this->offset_y_;
  if (x_data < 0 || x_data >= this->width_ || y_data < 0 || y_data >= this->height_)
    return false;
  const uint32_t width_8 = ((this->width_ + 7u) / 8u) * 8u;
  const uint32_t pos = x_data + y_data * width_8;
  return pgm_read_byte(this->data_ + (pos / 8u)) & (0x80 >> (pos % 8u));
}

const char *Glyph::get_char() const {
  return this->char_;
}

bool Glyph::compare_to(const char *str) const {
  // 1 -> this->char_
  // 2 -> str
  for (uint32_t i = 0;; i++) {
    if (this->char_[i] == '\0')
      return true;
    if (str[i] == '\0')
      return false;
    if (this->char_[i] > str[i])
      return false;
    if (this->char_[i] < str[i])
      return true;
  }
  // this should not happen
  return false;
}

int Glyph::match_length(const char *str) const {
  for (uint32_t i = 0;; i++) {
    if (this->char_[i] == '\0')
      return i;
    if (str[i] != this->char_[i])
      return 0;
  }
  // this should not happen
  return 0;
}

void Glyph::scan_area(int *x1, int *y1, int *width, int *height) const {
  *x1 = this->offset_x_;
  *y1 = this->offset_y_;
  *width = this->width_;
  *height = this->height_;
}

int Glyph::get_width() const {
  return this->width_;
}

int Glyph::get_height() const {
  return this->height_;
}

int Glyph::get_offset_x() const {
  return this->offset_x_;
}

int Glyph::get_offset_y() const {
  return this->offset_y_;
}

}  // namespace display
}  // namespace esphome
