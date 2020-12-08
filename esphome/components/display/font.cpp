#include "font.h"

namespace esphome {
namespace display {

Font::Font(std::vector<Glyph> &&glyphs, int baseline, int bottom)
    : glyphs_(std::move(glyphs)), baseline_(baseline), bottom_(bottom) {
}

int Font::match_next_glyph(const char *str, int *match_length) {
  int lo = 0;
  int hi = this->glyphs_.size() - 1;
  while (lo != hi) {
    int mid = (lo + hi + 1) / 2;
    if (this->glyphs_[mid].compare_to(str))
      lo = mid;
    else
      hi = mid - 1;
  }
  *match_length = this->glyphs_[lo].match_length(str);
  if (*match_length <= 0)
    return -1;
  return lo;
}

void Font::measure(const char *str, int *width, int *x_offset, int *baseline, int *height) {
  *baseline = this->baseline_;
  *height = this->bottom_;
  int i = 0;
  int min_x = 0;
  bool has_char = false;
  int x = 0;
  while (str[i] != '\0') {
    int match_length;
    int glyph_n = this->match_next_glyph(str + i, &match_length);
    if (glyph_n < 0) {
      // Unknown char, skip
      if (!this->get_glyphs().empty())
        x += this->get_glyphs()[0].get_width();
      i++;
      continue;
    }

    const Glyph &glyph = this->glyphs_[glyph_n];
    if (!has_char)
      min_x = glyph.get_offset_x();
    else
      min_x = std::min(min_x, x + glyph.get_offset_x());
    x += glyph.get_width() + glyph.get_offset_x();

    i += match_length;
    has_char = true;
  }
  *x_offset = min_x;
  *width = x - min_x;
}

const std::vector<Glyph> &Font::get_glyphs() const {
  return this->glyphs_;
}

}  // namespace display
}  // namespace esphome
