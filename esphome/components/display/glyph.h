#pragma once

#include <cstdint>

namespace esphome {
namespace display {

class Glyph {
 public:
  Glyph(const char *a_char, const uint8_t *data_start, uint32_t offset, int offset_x, int offset_y, int width,
        int height);

  bool get_pixel(int x, int y) const;

  const char *get_char() const;

  bool compare_to(const char *str) const;

  int match_length(const char *str) const;

  void scan_area(int *x1, int *y1, int *width, int *height) const;

  int get_width() const;

  int get_height() const;

  int get_offset_x() const;

  int get_offset_y() const;

 protected:
  const char *char_;
  const uint8_t *data_;
  int offset_x_;
  int offset_y_;
  int width_;
  int height_;
};

}  // namespace display
}  // namespace esphome
