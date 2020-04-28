#include "dimmer_output.h"
#include "esphome/core/log.h"

#ifdef ARDUINO_ARCH_ESP32

namespace esphome {
namespace dimmer_output {

static const char *TAG = "dimmer_output.output";
DimmerOutputComponent *DimmerOutputComponent::instance_ = nullptr;
constexpr uint32_t TIME_HALF_PERIOD_US = 10000;

void DimmerOutputComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up DimmerOutputComponent...");
  instance_ = this;
  this->pin_phase_in_->setup();
  this->pin_phase_cut_->setup();
  this->pin_phase_cut_->digital_write(false);
  this->pin_phase_cut_isr_ = this->pin_phase_cut_->to_isr();
  this->pin_phase_in_isr_ = this->pin_phase_in_->to_isr();
  this->pin_phase_in_->attach_interrupt(zero_crossing, this, CHANGE);
  this->timer_ = timerBegin(this->timer_id_, 80, true);
  timerAttachInterrupt(this->timer_, timer_expired, true);
}

void DimmerOutputComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Dimmer:");
  LOG_PIN("  PHASE_IN Pin: ", this->pin_phase_in_);
  LOG_PIN("  PHASE CUT Pin: ", this->pin_phase_cut_);
}

void DimmerOutputComponent::write_state(float state) {
  this->level_ = state;
  if (this->level_ <= 0.01f) {
    this->pulse_state_ = PULSE_WAIT_PHASE;
    instance_->pin_phase_cut_->digital_write(false);
    this->on_pulse_time_ = 0;
    this->off_pulse_time_ = 0;
  } else if (this->level_ >= 0.99f) {
    this->pulse_state_ = PULSE_WAIT_PHASE;
    instance_->pin_phase_cut_->digital_write(true);
    this->on_pulse_time_ = 0;
    this->off_pulse_time_ = 0;
  }
  else {
    this->on_pulse_time_ = std::max(this->level_ * (TIME_HALF_PERIOD_US - this->phase_delay_), 5.0f);
    this->off_pulse_time_ = (TIME_HALF_PERIOD_US - this->phase_delay_) - this->on_pulse_time_;
  }
}

void IRAM_ATTR DimmerOutputComponent::zero_crossing(DimmerOutputComponent *arg) {
  if ((arg->pulse_state_ == PULSE_WAIT_PHASE) && (arg->on_pulse_time_ > 0) && (arg->off_pulse_time_ > 0)) {
    arg->pulse_state_ = PULSE_OFF;
    ISRTimerRestart(instance_->timer_);
    ISRTimerAlarmWrite(instance_->timer_, instance_->off_pulse_time_, false);
    ISRTimerAlarmEnable(instance_->timer_);
  }
}

void IRAM_ATTR DimmerOutputComponent::timer_expired() {
  if (instance_->pulse_state_ == PULSE_OFF) {
    instance_->pulse_state_ = PULSE_ON;
    instance_->pin_phase_cut_isr_->digital_write(true);
    ISRTimerRestart(instance_->timer_);
    ISRTimerAlarmWrite(instance_->timer_, instance_->on_pulse_time_, false);
    ISRTimerAlarmEnable(instance_->timer_);
  }
  else if (instance_->pulse_state_ == PULSE_ON) {
    instance_->pin_phase_cut_isr_->digital_write(false);
    instance_->pulse_state_ = PULSE_WAIT_PHASE;
  }
}

}  // namespace dimmer_output
}  // namespace esphome

#endif
