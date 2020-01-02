#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"
#include <string>

namespace esphome {
namespace sht25 {

/// This class implements support for the SHT25temperature+humidity i2c sensor.
class SHT25Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { temperature_sensor_ = temperature_sensor; }
  void set_humidity_sensor(sensor::Sensor *humidity_sensor) { humidity_sensor_ = humidity_sensor; }

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void update() override;

 protected:
  enum class SHT25State {
    IN_RESET,
    READ_TEMP,
    WAIT_TEMP,
    READ_HUMID,
    WAIT_HUMID,
  };
  bool read_serial_number_(std::string& serial_number);
  bool check_crc_(const uint8_t* data, uint8_t len, uint8_t checksum) const;
  float calc_temperature_(const uint8_t* raw_temp) const;
  float calc_humidity_(const uint8_t* raw_humid) const;

  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *humidity_sensor_{nullptr};
  SHT25State sht25_state_{SHT25State::READ_TEMP};
  float temperature_{0.0f};
  float humidity_{0.0f};
  uint32_t retry_count_{0};
};

}  // namespace sht25
}  // namespace esphome
