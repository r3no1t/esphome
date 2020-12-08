#pragma once

#include "drawable.h"

namespace esphome {
namespace display {

class DisplayBuffer;
class Image;

class Icon : public Drawable {
 public:
  void lazy_init(DisplayBuffer &display) override;
  void draw_component(DisplayBuffer &display) override;
  void set_image(Image* image);
  void crop(int x1, int y1, int x2, int y2);

 private:
  Image* image_{nullptr};
  int x1_{0};
  int y1_{0};
  int x2_{0};
  int y2_{0};
};

}  // namespace display
}  // namespace esphome
