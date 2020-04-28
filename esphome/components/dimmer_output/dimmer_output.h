#pragma once

#include "esphome/core/component.h"
#include "esphome/core/esphal.h"
#include "esphome/components/output/float_output.h"

#ifdef ARDUINO_ARCH_ESP32

namespace esphome {
namespace dimmer_output {

/// dimmer_output float output component.
class DimmerOutputComponent : public output::FloatOutput, public Component {
 public:
  void set_pin_phase_in(GPIOPin *pin) { pin_phase_in_ = pin; }
  void set_pin_phase_cut(GPIOPin *pin) { pin_phase_cut_ = pin; }
  void set_hw_timer_id(uint32_t timer_id) { timer_id_ = timer_id; }
  void set_phase_delay(uint32_t phase_delay) { phase_delay_ = phase_delay; }
  /// Setup the dimmer_output.
  void setup() override;
  void dump_config() override;
  /// HARDWARE setup_priority
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

 protected:
  enum PulseState
  {
    PULSE_WAIT_PHASE,
    PULSE_OFF,
    PULSE_ON,
  };
  static void IRAM_ATTR zero_crossing(DimmerOutputComponent *arg);
  static void IRAM_ATTR timer_expired();
  void write_state(float state) override;

  static DimmerOutputComponent *instance_;
  GPIOPin *pin_phase_in_ = nullptr;
  GPIOPin *pin_phase_cut_ = nullptr;
  ISRInternalGPIOPin *pin_phase_cut_isr_ = nullptr;
  ISRInternalGPIOPin *pin_phase_in_isr_ = nullptr;
  hw_timer_t* timer_ = nullptr;
  PulseState pulse_state_ = PULSE_WAIT_PHASE;
  float level_ = 0.0f;
  uint32_t on_pulse_time_ = 0;
  uint32_t off_pulse_time_ = 0;
  uint32_t timer_id_ = 0;
  uint32_t phase_delay_ = 0;
};

}  // namespace dimmer_output
}  // namespace esphome

#endif
