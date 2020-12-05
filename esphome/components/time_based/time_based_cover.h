#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/cover/cover.h"

namespace esphome {
namespace time_based {

class TimeBasedCover : public cover::Cover, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override;

  Trigger<> *get_open_trigger() const { return this->open_trigger_; }
  Trigger<> *get_close_trigger() const { return this->close_trigger_; }
  Trigger<> *get_stop_trigger() const { return this->stop_trigger_; }
  void set_open_duration(uint32_t open_duration) { this->open_duration_ = open_duration; }
  void set_close_duration(uint32_t close_duration) { this->close_duration_ = close_duration; }
  void set_tilt_duration(uint32_t tilt_duration) { this->tilt_duration_ = tilt_duration; }
  cover::CoverTraits get_traits() override;
  void set_has_built_in_endstop(bool value) { this->has_built_in_endstop_ = value; }
  void set_assumed_state(bool value) { this->assumed_state_ = value; }
  void set_publish_interval(uint32_t interval) { this->publish_interval_ = interval; }
  void set_dir_change_delay(uint32_t delay) { this->dir_change_delay_ = delay; }
  void set_motor_sensor(binary_sensor::BinarySensor *motor_sensor) { this->motor_sensor_ = motor_sensor; }

 protected:
  void control(const cover::CoverCall &call) override;
  void stop_prev_trigger_();
  bool is_at_position_target_() const;
  void start_direction_(cover::CoverOperation dir);
  void stop_direction_();
  void recompute_position_();
  bool supports_tilt_() const { return this->tilt_duration_ != 0; }
  void update_tilt_target_(const cover::CoverCall &call);
  bool is_at_tilt_target_() const;
  cover::CoverOperation evaluate_direction_();

  Trigger<> *open_trigger_{new Trigger<>()};
  uint32_t open_duration_;
  Trigger<> *close_trigger_{new Trigger<>()};
  uint32_t close_duration_;
  Trigger<> *stop_trigger_{new Trigger<>()};
  uint32_t tilt_duration_;

  Trigger<> *prev_command_trigger_{nullptr};
  uint32_t last_recompute_time_{0};
  uint32_t last_publish_time_{0};
  uint32_t motor_on_time{0};
  float target_position_{0};
  float target_tilt_{0};
  bool has_built_in_endstop_{false};
  bool assumed_state_{false};
  uint32_t publish_interval_{1000};
  uint32_t dir_change_delay_{0};
  uint32_t start_next_operation_time_{0};
  cover::CoverOperation delayed_operation_{cover::COVER_OPERATION_IDLE};
  float tilt_delta_{0};
  float position_delta_{0};
  bool perform_stop_{0};
  binary_sensor::BinarySensor *motor_sensor_{nullptr};
};

}  // namespace time_based
}  // namespace esphome
