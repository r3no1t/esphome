#include "drawable.h"
#include "display_buffer.h"

namespace esphome {
namespace display {

void Drawable::draw(DisplayBuffer &display) {
  if (!this->initialized_) { this->initialized_ = true; this->display_ = &display; lazy_init(display); }
  if (this->visible_) {
    draw_component(display);
    for(Drawable *c : this->childs_) c->draw(display);
    for(Drawable *c : this->container_childs_) c->draw(display);
  }
}

void Drawable::set_parent(Drawable *parent) {
  this->parent_ = parent;
}

void Drawable::set_position(Position pos) {
  this->pos_ = pos;
}

void Drawable::add_child(Drawable *child) {
  childs_.push_back(child);
  child->set_parent(this);
  child->background_ = this->background_;
}

void Drawable::add_container_child(Drawable *child) {
  container_childs_.push_back(child);
  child->set_parent(this);
  child->background_ = this->background_;
}

void Drawable::set_visible(bool visible) {
  this->visible_ = visible;
  for(Drawable *c : this->childs_) c->set_visible(visible);
  for(Drawable *c : this->container_childs_) c->set_visible(visible);
}

void Drawable::set_background(ColorRGB color) {
  this->background_ = color;
}

void Drawable::add_on_press_callback(std::function<void(bool)> &&callback) {
  this->state_callback_.add(std::move(callback));
}

Position Drawable::get_pos() {
  return (this->parent_ == nullptr) ? this->pos_ : add_pos_to(this->parent_->get_pos());
}

bool Drawable::is_visible() {
  return this->visible_;
}

void Drawable::refresh() {
  if ((this->display_ != nullptr) && this->visible_) {
    this->draw_component(*display_);
    this->display_->repaint();
  }
}

Position Drawable::add_pos_to(const Position& pos) const {
  return {pos.x + this->pos_.x,
          pos.y + this->pos_.y,
          std::min(this->pos_.width, pos.width - this->pos_.x),
          std::min(this->pos_.height, pos.height - this->pos_.y) };
}

}  // namespace display
}  // namespace esphome
