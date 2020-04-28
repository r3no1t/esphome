#include "adc121s021.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

#ifdef ARDUINO_ARCH_ESP32

namespace esphome {
namespace adc121s021 {

static const char *TAG = "adc121s021";

ADCSensor *ADCSensor::instance_ = nullptr;

void ADCSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ADC121S021 '%s'...", this->name_.c_str());
  instance_ = this;

  this->clk_pin_->setup();
  this->cs_pin_->setup();
  this->data_pin_->setup();
  this->clk_pin_isr_ = this->clk_pin_->to_isr();
  this->cs_pin_isr_ = this->cs_pin_->to_isr();
  this->data_pin_isr_ = this->data_pin_->to_isr();
  this->clk_pin_->digital_write(true);
  this->cs_pin_->digital_write(true);

  this->filter_ = new LowpassFilter(this->filter_frequency_hz_, this->sample_time_us_);
  this->num_samples_ = (this->sample_duration_ms_ * 1000) / this->sample_time_us_;
  this->ring_buffer_ = new int32_t[this->num_samples_]{};
  this->ring_buffer_fil_ = new int32_t[this->num_samples_]{};

  this->timer_ = timerBegin(this->timer_id_, 80, true);
  timerAttachInterrupt(this->timer_, poll_sensor, true);
  timerAlarmWrite(this->timer_, this->sample_time_us_, true);
  timerAlarmEnable(this->timer_);
}

void ADCSensor::dump_config() {
  LOG_SENSOR("", "ADC121S021", this);
  LOG_PIN("  CLK Pin: ", this->clk_pin_);
  LOG_PIN("  CS Pin: ", this->cs_pin_);
  LOG_PIN("  DATA Pin: ", this->data_pin_);
  LOG_UPDATE_INTERVAL(this);
}

float ADCSensor::get_setup_priority() const {
  return setup_priority::DATA;
}

void ADCSensor::update() {
  this->lock_ = true;
  int32_t avg = 0;
  int32_t sum = 0;
  float sample = 0;
  this->filter_->init(this->ring_buffer_);
  for(uint32_t i = 0; i < this->num_samples_; ++i) {
    this->ring_buffer_fil_[i] = this->filter_->filter(this->ring_buffer_[i]);
    avg += this->ring_buffer_fil_[i];
  }
  avg = avg / this->num_samples_;
  for(uint32_t i = 0; i < this->num_samples_; ++i) {
    sum += (this->ring_buffer_fil_[i]-avg) * (this->ring_buffer_fil_[i]-avg);
  }
  sample = sqrt(sum / this->num_samples_);
  this->publish_state(sample);
  this->lock_ = false;
}

void IRAM_ATTR ADCSensor::poll_sensor() {
  if (not instance_->lock_) {
    if (instance_->pause_) {
      instance_->pause_ = false;
      ISRTimerRestart(instance_->timer_);
      ISRTimerAlarmWrite(instance_->timer_, instance_->sample_time_us_, true);
      ISRTimerAlarmEnable(instance_->timer_);
    }
    instance_->ring_buffer_[instance_->write_index_++] = instance_->getADCValue();
    if (instance_->write_index_ == instance_->num_samples_) {
      instance_->write_index_ = 0;
      ISRTimerRestart(instance_->timer_);
      ISRTimerAlarmWrite(instance_->timer_, instance_->sample_pause_time_ms_ * 1000, true);
      ISRTimerAlarmEnable(instance_->timer_);
      instance_->pause_ = true;
    }
  }
}

uint32_t IRAM_ATTR ADCSensor::getADCValue() {
  int16_t i = 0;
  uint32_t data = 0;
  this->cs_pin_isr_->digital_write(false);
  delay_nanos(40);
  for (i = 14; i >= 0; --i) {
    this->clk_pin_isr_->digital_write(false);
    delay_nanos(40);
    if (this->data_pin_isr_->digital_read()) data = (data | 1<<i);
    this->clk_pin_isr_->digital_write(true);
    delay_nanos(40);
  }
  this->clk_pin_isr_->digital_write(false);
  delay_nanos(40);
  this->clk_pin_isr_->digital_write(true);
  delay_nanos(40);
  this->cs_pin_isr_->digital_write(true);
  return data;
}

}  // namespace adc121s021
}  // namespace esphome

#endif

