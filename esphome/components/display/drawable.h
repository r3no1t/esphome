#pragma once

#include "esphome/core/helpers.h"
#include <functional>
#include <vector>

namespace esphome {
namespace display {

class DisplayBuffer;

struct ColorRGB {
  int r;
  int g;
  int b;
  int to_16bit() const { return (((r >> 3) & 0x1F) << 11) + (((g >> 2) & 0x3F) << 6) + ((b >> 3) & 0x1F); }
};

struct Position {
  int x;
  int y;
  int width;
  int height;
};

class Drawable {
public:
  virtual ~Drawable() = default;
  virtual void draw_component(DisplayBuffer &display) = 0;
  virtual void lazy_init(DisplayBuffer &display) {}
  void draw(DisplayBuffer &display);
  void set_parent(Drawable *parent);
  void set_position(Position pos);
  void add_child(Drawable *child);
  void add_container_child(Drawable *child);
  void set_visible(bool visible);
  void set_background(ColorRGB color);
  void add_on_press_callback(std::function<void(bool)> &&callback);

protected:
  Position get_pos();
  bool is_visible();
  void refresh();

  CallbackManager<void(bool)> state_callback_{};
  ColorRGB background_{-1,-1,-1};

private:
  Position add_pos_to(const Position& pos) const;

  Drawable *parent_{nullptr};
  Position pos_;
  std::vector<Drawable*> childs_;
  std::vector<Drawable*> container_childs_;
  bool initialized_{false};
  bool visible_{true};
  DisplayBuffer* display_{nullptr};
};

}  // namespace display
}  // namespace esphome
