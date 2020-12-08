#pragma once

#include "display_buffer.h"
#include "esphome/core/automation.h"

namespace esphome {
namespace display {

class DisplayPage {
 public:
  DisplayPage(const display_writer_t &writer);
  void show();
  void show_next();
  void show_prev();
  void set_parent(DisplayBuffer *parent);
  void set_prev(DisplayPage *prev);
  void set_next(DisplayPage *next);
  const display_writer_t &get_writer() const;

 protected:
  DisplayBuffer *parent_;
  display_writer_t writer_;
  DisplayPage *prev_{nullptr};
  DisplayPage *next_{nullptr};
};

}  // namespace display
}  // namespace esphome
