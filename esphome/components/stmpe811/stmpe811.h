#pragma once

#include "esphome/core/component.h"
#include "esphome/components/spi/spi.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include "stmpe811_defines.h"

namespace esphome {
namespace stmpe811 {


class STMPE811Component : public Component, public touchscreen::Touchscreen,
                          public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW,
                                                spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_1MHZ> {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;

  void loop() override;
  void set_backlight_timeout(uint32_t timeout) { this->backlight_timeout_ = timeout; }
  void set_limit_x_min(uint32_t limit) { this->limit_x_min_ = limit; }
  void set_limit_x_max(uint32_t limit) { this->limit_x_max_ = limit; }
  void set_limit_y_min(uint32_t limit) { this->limit_y_min_ = limit; }
  void set_limit_y_max(uint32_t limit) { this->limit_y_max_ = limit; }

 protected:
  enum class State {
    READ_DEVICE_ID,
    INITIALIZE,
    WAIT_FOR_TOUCH,
    SWITCH_ON_BACKLIGHT,
    WAIT_BACKLIGHT_TOUCH_END,
    READ_TOUCH_START,
    READ_TOUCH,
    WAIT_TOUCH_END,
  };
  void initialize();
  bool report_time_expired();
  bool backlight_time_expired();
  bool touch_debounce_expired();
  void write_register(uint8_t reg, uint8_t val);
  void write_register16(uint8_t reg, uint16_t val);
  uint8_t read_register(uint8_t reg);
  bool read_avg_touch(uint16_t& x, uint16_t& y, uint8_t& z);
  void read_data(uint16_t *x, uint16_t *y, uint8_t *z);
  uint16_t get_version();
  bool touched();
  bool buffer_empty();
  uint8_t buffer_size();

  bool initialized_{false};
  uint32_t backlight_timeout_{0};
  bool backlight_on_{true};
  uint32_t limit_x_min_{460};
  uint32_t limit_x_max_{3830};
  uint32_t limit_y_min_{250};
  uint32_t limit_y_max_{3810};
  uint16_t x{0};
  uint16_t y{0};
  uint8_t z{0};
  State state_{State::READ_DEVICE_ID};
  uint32_t last_touched_{0};
  uint32_t last_reported_{0};

};

}  // namespace stmpe811
}  // namespace esphome
