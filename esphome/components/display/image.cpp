#include "image.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace display {

Image::Image(const uint8_t *data_start, int width, int height)
    : width_(width), height_(height), data_start_(data_start) {
}

int Image::get_color_pixel(int x, int y) const {
  if (x < 0 || x >= this->width_ || y < 0 || y >= this->height_)
    return false;

  const uint32_t bit_pos = (y * this->width_ + x) * this->num_color_bits_;
  const uint32_t byte_pos = bit_pos / 8;
  int rgb_id = (pgm_read_byte(this->data_start_ + byte_pos) >> (bit_pos % 8)) & ((1 << this->num_color_bits_) - 1);
  if (((bit_pos % 8) + this->num_color_bits_) > 8) {
    rgb_id |= (pgm_read_byte(this->data_start_ + byte_pos + 1) << (8 - bit_pos % 8)) & ((1 << this->num_color_bits_) - 1);
  }
  if (rgb_id < colors_.size()) {
    return colors_[rgb_id];
  }
  return 0;
}

bool Image::get_pixel(int x, int y) const {
  if (x < 0 || x >= this->width_ || y < 0 || y >= this->height_)
    return false;
  const uint32_t width_8 = ((this->width_ + 7u) / 8u) * 8u;
  const uint32_t pos = x + y * width_8;
  return pgm_read_byte(this->data_start_ + (pos / 8u)) & (0x80 >> (pos % 8u));
}

int Image::get_width() const {
  return this->width_;
}

int Image::get_height() const {
  return this->height_;
}

bool Image::use_color() const {
  return this->colors_.size() > 0;
}

void Image::set_colors(int num_color_bits, std::vector<int> colors) {
  this->num_color_bits_ = num_color_bits;
  this->colors_ = colors;
}

}  // namespace display
}  // namespace esphome
