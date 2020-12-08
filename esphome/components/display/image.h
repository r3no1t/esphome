#pragma once

#include <vector>

namespace esphome {
namespace display {

class Image {
 public:
  Image(const uint8_t *data_start, int width, int height);
  bool get_pixel(int x, int y) const;
  int get_color_pixel(int x, int y) const;
  int get_width() const;
  int get_height() const;
  bool use_color() const;
  void set_colors(int num_color_bits, std::vector<int> colors);

 protected:
  int width_;
  int height_;
  const uint8_t *data_start_;
  std::vector<int> colors_;
  int num_color_bits_;
};

}  // namespace display
}  // namespace esphome
