#pragma once

#include "esphome/core/component.h"
#include "esphome/core/esphal.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace adc121s021 {


class LowpassFilter {
 public:
  LowpassFilter(int32_t frequency_hz, int32_t sample_time_us)
    : frequency_(frequency_hz)
    , sample_time_(sample_time_us / 1000000.0f)
    , alpha_((1.0f - exp(-sample_time_ * frequency_)) * 1000000.0f) {
  }

  float filter(int32_t input) {
    return output_ += ((float)(input - output_) * alpha_) / 1000000.0f;
  }

  /// calculates filter start value, per default takes average of first 10 samples
  void init(int32_t* samples, uint32_t num_samples = 10) {
    int32_t sum = 0;
    for(uint32_t i = 0; i < num_samples; ++i)
    {
        sum += samples[i];
    }
    output_ = sum / num_samples;
  }

 private:
  const float frequency_;
  const float sample_time_;
  const float alpha_;
  int32_t output_;
};


class ADCSensor : public sensor::Sensor, public PollingComponent {
 public:
  void set_clk_pin(GPIOPin *pin) { clk_pin_ = pin; }
  void set_cs_pin(GPIOPin *pin) { cs_pin_ = pin; }
  void set_data_pin(GPIOPin* pin) { data_pin_ = pin; }
  void set_hw_timer_id(uint32_t timer_id) { timer_id_ = timer_id; }
  void set_sample_time(uint32_t sample_time_us) { sample_time_us_ = sample_time_us; }
  void set_sample_duration(uint32_t sample_duration) { sample_duration_ms_ = sample_duration; }
  void set_sample_pause(uint32_t sample_pause_time) { sample_pause_time_ms_ = sample_pause_time; }
  void set_filter_frequency(uint32_t filter_frequency) { filter_frequency_hz_ = filter_frequency; }

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void update() override;

 protected:
  static void IRAM_ATTR poll_sensor();
  uint32_t IRAM_ATTR getADCValue();
  inline void IRAM_ATTR delay_nanos(const uint32_t nanos) const {
    for(uint32_t i = 0; i < nanos/4; ++i) NOP();
  }

  static ADCSensor *instance_;

  GPIOPin *clk_pin_ = nullptr;
  GPIOPin *cs_pin_ = nullptr;
  GPIOPin *data_pin_ = nullptr;
  ISRInternalGPIOPin *clk_pin_isr_ = nullptr;
  ISRInternalGPIOPin *cs_pin_isr_ = nullptr;
  ISRInternalGPIOPin *data_pin_isr_ = nullptr;

  hw_timer_t* timer_ = nullptr;
  uint32_t sample_index_ = 0;
  uint32_t write_index_ = 0;
  uint32_t num_samples_ = 0;
  int32_t *ring_buffer_ = nullptr;
  int32_t *ring_buffer_fil_ = nullptr;
  uint32_t timer_id_ = 0;
  LowpassFilter *filter_ = nullptr;
  uint32_t sample_time_us_ = 0;
  uint32_t sample_duration_ms_ = 0;
  uint32_t sample_pause_time_ms_ = 0;
  uint32_t filter_frequency_hz_ = 0;
  bool pause_ = false;
  bool lock_ = false;
};

}  // namespace adc121s021
}  // namespace esphome
