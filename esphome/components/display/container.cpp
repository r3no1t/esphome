#include "container.h"
#include "display_buffer.h"
#include "font.h"
#include "image.h"

namespace esphome {
namespace display {

void Container::lazy_init(DisplayBuffer &display) {
  if (this->type_ == TABBED_BOTTOM) {
    auto* touch = display.get_touchscreen();

    this->calc_tab_sizes(display);
    if (touch != nullptr) touch->add_touch_handler([&display,this](esphome::touchscreen::TouchEvent e){
      if (!this->is_visible()) return;
      bool changed = false;
      if (e.p == esphome::touchscreen::TouchPhase::START) {
        this->start_gesture_detection(e.x, e.y);
      }
      else {
        switch(this->detected_gesture(e.x, e.y)) {
        case SWIPE_RIGHT:
          changed = this->select_next_container();
          break;
        case SWIPE_LEFT:
          changed = this->select_previous_container();
          break;
        default:
          break;
        }
      }
      for (Container *c : this->containers_) {
        Position pos = get_tab_pos(c);

        if ((e.x > pos.x && e.x < (pos.x + pos.width) && e.y > pos.y && e.y < (pos.y + pos.height)) &&
            (e.p == esphome::touchscreen::TouchPhase::START) &&
            (c != this->selected_container_)) {
          this->selected_container_->set_visible(false);
          this->selected_container_ = c;
          this->selected_container_->set_visible(true);
          changed = true;
        }
      }
      if (changed) {
        this->draw(display);
        display.repaint();
      }
    });
  }
}

void Container::draw_component(DisplayBuffer &display) {
//    ESP_LOGD("display", "tab:%s", tab_name_.c_str());
  Position pos{get_pos()};

  if (this->background_.r != -1)
    display.filled_rectangle(pos.x, pos.y, pos.width, pos.height, this->background_.to_16bit());
  if (this->type_ == TABBED_BOTTOM) {
    display_tabs_bottom(display);
    if (this->selected_container_ != nullptr) {
      this->selected_container_->draw(display);
    }
  }
  else {
    if (this->border_top_.r != -1)
      display.horizontal_line(pos.x, pos.y, pos.width, this->border_top_.to_16bit());
    if (this->border_bottom_.r != -1)
      display.horizontal_line(pos.x, pos.height - 2, pos.width, this->border_bottom_.to_16bit());
  }
}

void Container::add_container(Container *container){
  this->containers_.push_back(container);
  container->set_parent(this);
  if (this->selected_container_ == nullptr) {
    this->selected_container_ = container;
  }
  else container->set_visible(false);
}

void Container::start_gesture_detection(uint32_t x, uint32_t y) {
  this->start_x_ = x;
  this->start_y_ = y;
}

uint32_t Container::detected_gesture(uint32_t x, uint32_t y) {
  if (this->start_x_ == 0xFFFF) return SWIPE_NONE;
  if (x > (this->start_x_ + 50)) {
    this->start_x_ = 0xFFFF; return SWIPE_RIGHT;
  }
  if ((this->start_x_ > 50) && (x < (this->start_x_ - 50))) {
    this->start_x_ = 0xFFFF; return SWIPE_LEFT;
  }
  return SWIPE_NONE;
}

bool Container::select_next_container() {
  for (int i = 0; i < this->containers_.size(); ++i) {
    if (this->containers_[i] == this->selected_container_ && i < (this->containers_.size() - 1)) {
      this->selected_container_->set_visible(false);
      this->selected_container_ = this->containers_[i + 1];
      this->selected_container_->set_visible(true);
      return true;
    }
  }
  return false;
}

bool Container::select_previous_container() {
  for (int i = 0; i < this->containers_.size(); ++i) {
    if (this->containers_[i] == this->selected_container_ && i > 0) {
      this->selected_container_->set_visible(false);
      this->selected_container_ = this->containers_[i - 1];
      this->selected_container_->set_visible(true);
      return true;
    }
  }
  return false;
}

void Container::display_tabs_bottom(DisplayBuffer &display) {
  Position pos{get_pos()};

  display.horizontal_line(pos.x, pos.y + pos.height - this->tab_height_, pos.width, this->border_bottom_.to_16bit());
  for (Container *c : this->containers_) {
    bool is_selected = c == this->selected_container_;
    Position pos = get_tab_pos(c);
    Image* image = is_selected ? c->image_selected_ : c->image_;

//      display.rectangle(pos.x, pos.y, pos.width, pos.height - 1, this->border_bottom_.to_16bit());
//      display.filled_rectangle(pos.x+1, is_selected ? pos.y : pos.y + 1, pos.width - 1, is_selected ? pos.height - 2 : pos.height - 3, this->background_.to_16bit());
    if (image != nullptr) {
      display.image(pos.x + 5, pos.y + (pos.height - image->get_height()) / 2, image);
      if (c->tab_name_.size() > 0) {
        display.print(pos.x + 10 + image->get_width(), pos.y + (pos.height - c->text_height_) / 2, c->font_, this->color_.to_16bit(), c->tab_name_.c_str());
      }
    }
    else if (c->tab_name_.size() > 0) {
      display.print(pos.x + 5, pos.y + (pos.height - c->text_height_) / 2, this->font_, this->color_.to_16bit(), this->tab_name_.c_str());
    }
  }
}

void Container::calc_tab_sizes(DisplayBuffer &display) {
  Position pos{get_pos()};
  int total_tab_width = 0;
  int next_tab_pos = 0;

  for (Container *c : this->containers_) {
    int text_width = 0;
    int text_height = 0;

    if (c->tab_name_.size() > 0 && c->font_ != nullptr) {
      int x,y;

      display.get_text_bounds(0, 0, c->tab_name_.c_str(), c->font_, TextAlign::TOP_LEFT, &x, &y, &text_width, &text_height);
      c->text_height_ = text_height;
      text_width += 10;
    }
    c->tab_width_ = text_width + ((c->image_ != nullptr) ? c->image_->get_width() + 5 : 0);
    total_tab_width += c->tab_width_;
  }
  // TODO use tab alignment config
  int offset = std::max(0, (pos.width - total_tab_width) / 2);
  for (Container *c : this->containers_) {
    c->tab_pos_ = next_tab_pos + offset;
    next_tab_pos += c->tab_width_;
    this->tab_height_ = std::max(this->tab_height_, std::max(c->text_height_, c->image_ != nullptr ? c->image_->get_height() : 0) + 10);
  }
}

Position Container::get_tab_pos(Container *c) {
  Position parent_pos{get_pos()};
  Position pos;

  pos.x = c->tab_pos_;
  pos.y = parent_pos.y + parent_pos.height - this->tab_height_;
  pos.width = c->tab_width_;
  pos.height = this->tab_height_;
  return pos;
}

}  // namespace display
}  // namespace esphome
