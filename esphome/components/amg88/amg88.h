#pragma once

#include <array>
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace amg88 {

enum AMG88FrameRate {
  AMG8833_FRAME_RATE_10_FPS = 0,
  AMG8833_FRAME_RATE_1_FPS = 1,
};

/// This class implements support for the AMG88 infrared array i2c sensor.
class AMG88Component : public text_sensor::TextSensor, public PollingComponent, public i2c::I2CDevice {
 public:
  void set_frame_rate(AMG88FrameRate frame_rate) { this->frame_rate_ = frame_rate; }
  void set_min_change_pixel(uint32_t min_change_pixels) { this->min_change_pixels_ = min_change_pixels; };
  void set_min_change_degrees(float min_change_degrees) { this->min_change_degrees_ = min_change_degrees; };
  void set_offset_degrees(int32_t offset_degrees) { this->offset_degrees_ = offset_degrees; };
  void set_send_initial_state_(bool send_initial_state) { this->send_initial_state_ = send_initial_state; };

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void update() override;

 protected:
  static const uint8_t NUMBER_OF_PIXELS = 64;
  static const uint8_t FILTER_SIZE = 10;
  static const uint32_t MAX_RETRY = 5;
  using PixelData = std::array<uint8_t, NUMBER_OF_PIXELS>;

  bool init_sensor();
  bool read_sensor();
  void init_pixels();
  void send_pixel_state();
  bool update_pixels();

  PixelData pixels_{};
  std::array<PixelData, NUMBER_OF_PIXELS> pixels_buffer_{};
  uint32_t retry_count_{0};
  AMG88FrameRate frame_rate_{AMG8833_FRAME_RATE_10_FPS};
  uint32_t min_change_pixels_{1};
  float min_change_degrees_{0.25f};
  int32_t offset_degrees_{0};
  bool send_initial_state_{true};
  std::string sensor_string_{""};
  uint32_t write_index_{0};
  bool is_initialized_{false};
};

}  // namespace amg88
}  // namespace esphome
