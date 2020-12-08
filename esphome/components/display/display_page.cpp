#include "display_page.h"

namespace esphome {
namespace display {

DisplayPage::DisplayPage(const display_writer_t &writer) : writer_(writer) {
}

void DisplayPage::show() {
  this->parent_->show_page(this);
}

void DisplayPage::show_next() {
  this->next_->show();
}

void DisplayPage::show_prev() {
  this->prev_->show();
}

void DisplayPage::set_parent(DisplayBuffer *parent) {
  this->parent_ = parent;
}

void DisplayPage::set_prev(DisplayPage *prev) {
  this->prev_ = prev;
}

void DisplayPage::set_next(DisplayPage *next) {
  this->next_ = next;
}

const display_writer_t &DisplayPage::get_writer() const {
  return this->writer_;
}

}  // namespace display
}  // namespace esphome
