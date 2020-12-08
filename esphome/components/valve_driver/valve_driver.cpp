#include "valve_driver.h"
#include "esphome/core/log.h"

namespace esphome {
namespace valve_driver {

static const char *TAG = "valve_driver";
constexpr uint32_t REF_2_50_VOLT = 58;

void ValveDriver::dump_config() {
  ESP_LOGCONFIG(TAG, "ValveDriver:");
  ESP_LOGCONFIG(TAG, "  pin_clk_: %d", this->pin_clk_->get_pin());
  ESP_LOGCONFIG(TAG, "  pin_cs1_: %d", this->pin_cs1_->get_pin());
  ESP_LOGCONFIG(TAG, "  pin_cs2_: %d", this->pin_cs2_->get_pin());
  ESP_LOGCONFIG(TAG, "  pin_data_: %d", this->pin_data_->get_pin());
}

void ValveDriver::set_value(uint32_t channel, int32_t value) {
  ESP_LOGD(TAG, "output set value of channel: %d to %d", channel, value);
  if ((channel >= 0) && (channel < 12)) {
    this->new_value_[channel] = value;
  }
}

void ValveDriver::setup() {
  this->pin_clk_->setup();
  this->pin_clk_->digital_write(false);
  this->pin_cs1_->setup();
  this->pin_cs1_->digital_write(true);
  this->pin_cs2_->setup();
  this->pin_cs2_->digital_write(true);
  this->pin_data_->setup();
  this->pin_data_->digital_write(false);
}

void ValveDriver::update() {
  for (uint32_t i = 0; i < 12; ++i) {
    if (this->old_value_[i] != this->new_value_[i]) {
      this->old_value_[i] = this->new_value_[i];
      uint8_t newState = ((this->new_value_[i] < 0) ? 0 : (this->new_value_[i] > REF_2_50_VOLT) ? REF_2_50_VOLT : this->new_value_[i]) * 4; // 0 .. 10 Volt
      updateValves(i+1, newState);
    }
  }
}

void ValveDriver::updateValves(uint32_t valveId, int32_t valvePos)
{
    ESP_LOGD(TAG, "set valve %d to value %d", valveId, valvePos);
    delayMicroseconds(5);
    if(valveId <= 8) this->pin_cs1_->digital_write(false); else this->pin_cs2_->digital_write(false);
    delayMicroseconds(5);
    for (uint32_t i = 0; i < 8; ++i)
    {
        if (((valveId-1) % 8) == i)
          this->pin_data_->digital_write(true);
        else
          this->pin_data_->digital_write(false);
        delayMicroseconds(5);
        this->pin_clk_->digital_write(true);
        delayMicroseconds(5);
        this->pin_clk_->digital_write(false);
        delayMicroseconds(5);
    }
    for (uint32_t i = 0; i < 8; ++i)
    {
        if (valvePos & (0x80 >> i))
          this->pin_data_->digital_write(true);
        else
          this->pin_data_->digital_write(false);
        delayMicroseconds(5);
        this->pin_clk_->digital_write(true);
        delayMicroseconds(5);
        this->pin_clk_->digital_write(false);
        delayMicroseconds(5);
    }
    if(valveId <= 8) this->pin_cs1_->digital_write(true); else this->pin_cs2_->digital_write(true);
}

}  // namespace valve_driver
}  // namespace esphome
