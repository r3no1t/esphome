#pragma once

#include "drawable.h"
#include <string>
#include <vector>

namespace esphome {
namespace display {

class DisplayBuffer;
class Image;
class Font;

enum ContainerType {
  PLAIN,
  TABBED_TOP,
  TABBED_BOTTOM,
};

class Container : public Drawable {
  enum Gesture {
    SWIPE_NONE,
    SWIPE_RIGHT,
    SWIPE_LEFT,
  };

 public:
  void lazy_init(DisplayBuffer &display) override;
  void draw_component(DisplayBuffer &display) override;
  void add_container(Container *container);
  void set_type(ContainerType type){ this->type_ = type;}
  void set_font(Font *font){ this->font_ = font; }
  void set_tab_name(const char* text){ this->tab_name_ = text; }
  void set_image(Image* image){ this->image_ = image; }
  void set_image_selected(Image* image){ this->image_selected_ = image; }
  void set_color(ColorRGB color){ this->color_ = color; }
  void set_border_top(ColorRGB color){ this->border_top_ = color; }
  void set_border_bottom(ColorRGB color){ this->border_bottom_ = color; }
  void set_border_left(ColorRGB color){ this->border_left_ = color; }
  void set_border_right(ColorRGB color){ this->border_right_ = color; }

private:
  void start_gesture_detection(uint32_t x, uint32_t y);
  uint32_t detected_gesture(uint32_t x, uint32_t y);
  bool select_next_container();
  bool select_previous_container();
  void display_tabs_bottom(DisplayBuffer &display);
  void calc_tab_sizes(DisplayBuffer &display);
  Position get_tab_pos(Container *c);

  std::vector<Container*> containers_;
  ContainerType type_{PLAIN};
  Font* font_{nullptr};
  std::string tab_name_;
  Image* image_{nullptr};
  Image* image_selected_{nullptr};
  ColorRGB color_{-1,-1,-1};
  ColorRGB border_top_{-1,-1,-1};
  ColorRGB border_bottom_{-1,-1,-1};
  ColorRGB border_left_{-1,-1,-1};
  ColorRGB border_right_{-1,-1,-1};
  Container* selected_container_{nullptr};
  int tab_pos_{0};
  int tab_height_{0};
  int tab_width_{0};
  int text_height_{0};
  uint32_t start_x_{0xFFFF};
  uint32_t start_y_{0xFFFF};
};

}  // namespace display
}  // namespace esphome
