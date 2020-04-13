#include "amg88.h"
#include "esphome/core/log.h"


namespace esphome {
namespace amg88 {

namespace {
const char *TAG = "amg88";

constexpr uint8_t REG_OPERATING_MODE = 0x00;
constexpr uint8_t REG_SOFTWARE_RESET = 0x01;
constexpr uint8_t REG_FRAME_RATE = 0x02;
constexpr uint8_t REG_INTERRUPT_CFG = 0x03;
constexpr uint8_t REG_PIXEL_OUTPUT_VALUE_START = 0x80;

constexpr uint8_t VALUE_SOFTWARE_RESET = 0b00111111;
constexpr uint8_t VALUE_NORMAL_MODE = 0b00000000;
constexpr uint8_t VALUE_INT_DISABLE = 0b00000000;
}

void AMG88Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up AMG88...");
  if (!this->init_sensor()) {
    this->mark_failed();
    return;
  }
}

void AMG88Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AMG88:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with AMG88 failed!");
  }
  LOG_UPDATE_INTERVAL(this);
}

float AMG88Component::get_setup_priority() const {
  return setup_priority::DATA;
}

void AMG88Component::update() {
  if(this->status_has_error()) {
    this->init_sensor();
  }
  if(this->read_sensor()) {
    if(!this->is_initialized_) {
      this->is_initialized_ = true;
      this->init_pixels();
      if(this->send_initial_state_) this->send_pixel_state();
    }
    else if(this->update_pixels()) {
      this->send_pixel_state();
    }
  }
}

void AMG88Component::init_pixels() {
  for(uint8_t i = 0; i < NUMBER_OF_PIXELS; ++i) {
    for(uint8_t k = 0; k < FILTER_SIZE; ++k) {
      this->pixels_buffer_[k][i] = this->pixels_[i];
    }
  }
}

bool AMG88Component::update_pixels() {
  uint32_t changed_pixels = 0;
  this->pixels_buffer_[this->write_index_] = this->pixels_;
  this->write_index_ = (this->write_index_ + 1) % FILTER_SIZE;
  for(uint8_t i = 0; (i < NUMBER_OF_PIXELS) && (changed_pixels < this->min_change_pixels_); ++i) {
    uint32_t avg = 0;
    for(uint8_t k = 0; k < FILTER_SIZE; ++k) avg += this->pixels_buffer_[k][i];
    avg = avg / FILTER_SIZE;
    if(abs((int32_t)avg - (int32_t)this->pixels_[i]) > (int32_t)(this->min_change_degrees_ * 4)) {
      ++changed_pixels;
    }
  }
  return changed_pixels >= this->min_change_pixels_;
}

void AMG88Component::send_pixel_state() {
  char tmp[5];
  this->sensor_string_ = "";
  for(auto pixel : this->pixels_) {
    sprintf(tmp, "%02X", pixel);
    sensor_string_.append(tmp);
  }
  this->publish_state(this->sensor_string_);
}

bool AMG88Component::init_sensor() {
  bool success = true;
  success = success && this->write_byte(REG_SOFTWARE_RESET, VALUE_SOFTWARE_RESET);
  success = success && this->write_byte(REG_OPERATING_MODE, VALUE_NORMAL_MODE);
  success = success && this->write_byte(REG_INTERRUPT_CFG, VALUE_INT_DISABLE);
  success = success && this->write_byte(REG_FRAME_RATE, this->frame_rate_);
  if(success && this->status_has_error()) {
    this->status_clear_error();
  }
  return success;
}

bool AMG88Component::read_sensor() {
  for(uint32_t i = 0; (i < NUMBER_OF_PIXELS) && !this->status_has_error(); ++i) {
    uint8_t valh = 0;
    uint8_t vall = 0;
    if(this->read_byte(REG_PIXEL_OUTPUT_VALUE_START + (i<<1), &vall) &&
       this->read_byte(REG_PIXEL_OUTPUT_VALUE_START + (i<<1) + 1, &valh)) {
      int16_t val = (valh << 4);
      val = (val >> 4) + vall + this->offset_degrees_;
      this->pixels_[i] = (uint8_t)((val < 0) ? 0 : (val > 0xFF) ? 0xFF : val);
      this->retry_count_ = 0;
    }
    else {
      ESP_LOGW(TAG, "read failed, retry");
      i -= 1; // retry same pixel read
      if(++this->retry_count_ > MAX_RETRY) {
        ESP_LOGE(TAG, "retry failed, reset sensor");
        this->retry_count_ = 0;
        this->status_set_error();
        return false;
      }
    }
  }
  return true;
}

}  // namespace amg88
}  // namespace esphome
