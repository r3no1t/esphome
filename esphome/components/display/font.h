#pragma once

#include "glyph.h"
#include <vector>

namespace esphome {
namespace display {

class Font {
 public:
  /** Construct the font with the given glyphs.
   *
   * @param glyphs A vector of glyphs, must be sorted lexicographically.
   * @param baseline The y-offset from the top of the text to the baseline.
   * @param bottom The y-offset from the top of the text to the bottom (i.e. height).
   */
  Font(std::vector<Glyph> &&glyphs, int baseline, int bottom);

  int match_next_glyph(const char *str, int *match_length);

  void measure(const char *str, int *width, int *x_offset, int *baseline, int *height);

  const std::vector<Glyph> &get_glyphs() const;

 protected:
  std::vector<Glyph> glyphs_;
  int baseline_;
  int bottom_;
};

}  // namespace display
}  // namespace esphome
