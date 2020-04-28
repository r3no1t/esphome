#include "stmpe811.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace stmpe811 {

static const char *TAG = "stmpe811";

void STMPE811Component::setup() {
  this->spi_setup();
}

void STMPE811Component::dump_config() {
  LOG_TOUCHSCREEN("", "STMPE811Component", this);
  ESP_LOGCONFIG(TAG, "  Rotation: %d", this->rotation_);
}

float STMPE811Component::get_setup_priority() const {
  return setup_priority::PROCESSOR;
}

void STMPE811Component::loop()
{
  switch(this->state_) {
  case State::READ_DEVICE_ID:
    if (this->get_version() == 0x811) this->state_ = State::INITIALIZE;
    ESP_LOGD(TAG, "READ_DEVICE_ID");
    break;
  case State::INITIALIZE:
    this->initialize();
    this->state_ = State::WAIT_FOR_TOUCH;
    ESP_LOGD(TAG, "WAIT_FOR_TOUCH");
    break;
  case State::WAIT_FOR_TOUCH:
    if (this->touched()) {
      this->state_ = ((this->backlight_timeout_ > 0) && !this->backlight_on_) ? State::SWITCH_ON_BACKLIGHT : State::READ_TOUCH_START;
      ESP_LOGD(TAG, "%s", ((this->backlight_timeout_ > 0) && !this->backlight_on_) ? "SWITCH_ON_BACKLIGHT" : "READ_TOUCH_START");
    }
    break;
  case State::SWITCH_ON_BACKLIGHT:
    this->last_touched_ = millis();
    this->write_register(STMPE_GPIO_SET_PIN, 0x04);
    this->backlight_on_ = true;
    this->state_ = State::WAIT_BACKLIGHT_TOUCH_END;
    ESP_LOGD(TAG, "WAIT_BACKLIGHT_TOUCH_END");
    break;
  case State::WAIT_BACKLIGHT_TOUCH_END:
    if (!(this->touched() || this->read_avg_touch(this->x, this->y, this->z))) {
      this->state_ = State::WAIT_FOR_TOUCH;
      this->last_touched_ = millis();
      ESP_LOGD(TAG, "WAIT_FOR_TOUCH");
    }
    break;
  case State::READ_TOUCH_START:
    if (this->touched() && this->read_avg_touch(this->x, this->y, this->z)) {
      this->state_ = State::READ_TOUCH;
      this->notify_listeners({esphome::touchscreen::TouchPhase::START, this->x, this->y, this->z});
      ESP_LOGD(TAG, "START: %d %d %d", this->x, this->y, this->z);
      ESP_LOGD(TAG, "READ_TOUCH_START");
    }
    break;
  case State::READ_TOUCH:
    if (this->touched()) {
      if (this->read_avg_touch(this->x, this->y, this->z) /*&& this->report_time_expired()*/) {
        this->notify_listeners({esphome::touchscreen::TouchPhase::CONTINUE, this->x, this->y, this->z});
        ESP_LOGD(TAG, "CONT: %d %d %d", this->x, this->y, this->z);
      }
    }
    else {
      this->last_touched_ = millis();
      this->state_ = State::WAIT_TOUCH_END;
      ESP_LOGD(TAG, "WAIT_TOUCH_END");
    }
    break;
  case State::WAIT_TOUCH_END:
    if (this->touched()) {
      this->state_ = State::READ_TOUCH;
      ESP_LOGD(TAG, "READ_TOUCH");
    }
    else if (this->touch_debounce_expired()) {
      this->notify_listeners({esphome::touchscreen::TouchPhase::END, this->x, this->y, this->z});
      ESP_LOGD(TAG, "END: %d %d %d", this->x, this->y, this->z);
      while (!this->buffer_empty()) this->read_data(&this->x, &this->y, &this->z);
      this->state_ = State::WAIT_FOR_TOUCH;
      ESP_LOGD(TAG, "WAIT_FOR_TOUCH");
    }
    break;
  }

  if (this->backlight_on_ && this->backlight_time_expired()) {
    this->write_register(STMPE_GPIO_CLR_PIN, 0x04);
    this->backlight_on_ = false;
    ESP_LOGD(TAG, "backlight off");
  }
}

void STMPE811Component::initialize() {
  this->write_register(STMPE_SYS_CTRL1, STMPE_SYS_CTRL1_RESET);
  delayMicroseconds(10);

  for (uint8_t i=0; i<65; i++) this->read_register(i);

  this->write_register(STMPE_SYS_CTRL2, 0x0);
  this->write_register(STMPE_TSC_CTRL, STMPE_TSC_CTRL_XYZ | STMPE_TSC_CTRL_EN);
  this->write_register16(STMPE_WDW_TR_X, 0xFFF);
  this->write_register16(STMPE_WDW_TR_Y, 0xFFF);
  this->write_register16(STMPE_WDW_BL_X, 0);
  this->write_register16(STMPE_WDW_BL_Y, 0);
  this->write_register(STMPE_INT_EN, STMPE_INT_EN_TOUCHDET);
  this->write_register(STMPE_ADC_CTRL1, STMPE_ADC_CTRL1_10BIT | (0x6 << 4));
  this->write_register(STMPE_ADC_CTRL2, STMPE_ADC_CTRL2_6_5MHZ);
  this->write_register(STMPE_TSC_CFG, STMPE_TSC_CFG_4SAMPLE | STMPE_TSC_CFG_DELAY_1MS | STMPE_TSC_CFG_SETTLE_5MS);
  this->write_register(STMPE_TSC_FRACTION_Z, 0x6);
  this->write_register(STMPE_FIFO_TH, 1);
  this->write_register(STMPE_FIFO_STA, STMPE_FIFO_STA_RESET);
  this->write_register(STMPE_FIFO_STA, 0);
  this->write_register(STMPE_TSC_I_DRIVE, STMPE_TSC_I_DRIVE_50MA);
  this->write_register(STMPE_INT_STA, 0xFF);
  this->write_register(STMPE_INT_CTRL, STMPE_INT_CTRL_POL_HIGH | STMPE_INT_CTRL_ENABLE);
  this->write_register(STMPE_GPIO_ALT_FUNCT, 0x04);
  this->write_register(STMPE_GPIO_DIR, 0x04);
  this->write_register(STMPE_GPIO_SET_PIN, 0x04);
}

bool STMPE811Component::report_time_expired() {
  return this->last_reported_ < (millis() - 100);
}

bool STMPE811Component::backlight_time_expired() {
  return (this->backlight_timeout_ > 0) && (millis() > this->backlight_timeout_ * 1000) &&
    (this->last_touched_ < (millis() - this->backlight_timeout_ * 1000));
}

bool STMPE811Component::touch_debounce_expired() {
  return this->last_touched_ < (millis() - 100);
}

bool STMPE811Component::touched() {
  return (this->read_register(STMPE_TSC_CTRL) & 0x80);
}

bool STMPE811Component::buffer_empty() {
  return (this->read_register(STMPE_FIFO_STA) & STMPE_FIFO_STA_EMPTY);
}

uint8_t STMPE811Component::buffer_size() {
  return this->read_register(STMPE_FIFO_SIZE);
}

uint16_t STMPE811Component::get_version() {
  uint16_t version = (this->read_register(0) << 8) | this->read_register(1);
  ESP_LOGD(TAG, "Version: 0x%04X", version);
  return version;
}

bool STMPE811Component::read_avg_touch(uint16_t& x, uint16_t& y, uint8_t& z) {
  uint32_t sumX = 0;
  uint32_t sumY = 0;
  uint32_t sumZ = 0;
  uint32_t count = 0;

  while (!this->buffer_empty() && (count < 5)) {
    ++count;
    read_data(&x, &y, &z);
    sumX += x;
    sumY += y;
    sumZ += z;
  }
  if (count > 0) {
    x = sumX/count;
    y = sumY/count;
    z = sumZ/count;
    return true;
  }
  return false;
}

void STMPE811Component::read_data(uint16_t *x, uint16_t *y, uint8_t *z) {
  uint8_t data[4];

  for (uint8_t i=0; i<4; i++) data[i] = this->read_register(0xD7);
  *y = (data[0] << 4) | (data[1] >> 4);
  *x = ((data[1] & 0x0F) << 8) | data[2];
  *z = data[3];

  if (*y < this->limit_y_min_) *y = this->limit_y_min_;
  if (*y > this->limit_y_max_) *y = this->limit_y_max_;
  if (*x < this->limit_x_min_) *x = this->limit_x_min_;
  if (*x > this->limit_x_max_) *x = this->limit_x_max_;
  *y = this->pixel_y_ - ((*y - limit_y_min_) * this->pixel_y_) / (this->limit_y_max_ - this->limit_y_min_);
  *x = ((*x - this->limit_x_min_) * this->pixel_x_) / (this->limit_x_max_ - this->limit_x_min_);

  if (this->buffer_empty()) this->write_register(STMPE_INT_STA, 0xFF);
}

uint8_t STMPE811Component::read_register(uint8_t reg) {
  uint8_t val = 0;

  this->enable();
  this->write_byte(0x80 | reg);
  this->write_byte(0x00);
  val = this->read_byte();
  this->disable();
  return val;
}

void STMPE811Component::write_register(uint8_t reg, uint8_t val) {
  this->enable();
  this->write_byte(reg);
  this->write_byte(val);
  this->disable();
}

void STMPE811Component::write_register16(uint8_t reg, uint16_t val) {
  this->enable();
  this->write_byte(reg);
  this->write_byte(val >> 8);
  this->write_byte(val & 0xFF);
  this->disable();
}

}  // namespace stmpe811
}  // namespace esphome

