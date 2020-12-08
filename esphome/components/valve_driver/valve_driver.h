#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace valve_driver {


class ValveDriver : public PollingComponent {
 public:
  void set_pin_clk(GPIOPin *pin) { pin_clk_ = pin; }
  void set_pin_cs1(GPIOPin *pin) { pin_cs1_ = pin; }
  void set_pin_cs2(GPIOPin *pin) { pin_cs2_ = pin; }
  void set_pin_data(GPIOPin *pin) { pin_data_ = pin; }

  void set_value(uint32_t channel, int32_t value);

  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  void updateValves(uint32_t valveId, int32_t valvePos);

  GPIOPin *pin_clk_ = nullptr;
  GPIOPin *pin_cs1_ = nullptr;
  GPIOPin *pin_cs2_ = nullptr;
  GPIOPin *pin_data_ = nullptr;
  int32_t new_value_[12] = { 0 };
  int32_t old_value_[12] = { 0 };
};

}  // namespace valve_driver
}  // namespace esphome
