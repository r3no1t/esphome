#include "time_based_cover.h"
#include "esphome/core/log.h"

namespace esphome {
namespace time_based {

static const char *TAG = "time_based.cover";

using namespace esphome::cover;

void TimeBasedCover::dump_config() {
  LOG_COVER("", "Time Based Cover", this);
  ESP_LOGCONFIG(TAG, "  Open Duration: %.1fs", this->open_duration_ / 1e3f);
  ESP_LOGCONFIG(TAG, "  Close Duration: %.1fs", this->close_duration_ / 1e3f);
}

void TimeBasedCover::setup() {
  auto restore = this->restore_state_();
  if (restore.has_value()) {
    restore->apply(this);
    ESP_LOGD(TAG, "restore POS:%.1fs TILT:%.1fs", this->position, this->tilt);
  } else {
    this->position = 0.5f;
    ESP_LOGD(TAG, "set POS:%.1fs TILT:%.1fs", this->position, this->tilt);
  }
  this->start_next_operation_time_ = millis() + this->dir_change_delay_;
  this->target_position_ = this->position;
  this->target_tilt_ = this->tilt;
  this->stop_direction_();
}

void TimeBasedCover::loop() {
  const uint32_t now = millis();

  if (this->perform_stop_) {
    this->perform_stop_ = false;
    this->stop_direction_();
    this->publish_state();
  }

  if (this->current_operation != COVER_OPERATION_IDLE || (this->motor_sensor_ != nullptr && this->motor_sensor_->state)) {
    // delay next direction change as long as there is a movement
    this->start_next_operation_time_ = now + this->dir_change_delay_;
  }

  if (this->current_operation != COVER_OPERATION_IDLE && this->motor_sensor_ != nullptr &&
      !this->motor_sensor_->state && ((this->motor_on_time + 500) <  now)) {
    // motor stopped, update end position
    if (this->current_operation == COVER_OPERATION_CLOSING) {
      this->position = COVER_CLOSED;
      this->tilt = COVER_CLOSED;
    }
    else if (this->current_operation == COVER_OPERATION_OPENING) {
      this->position = COVER_OPEN;
      this->tilt = COVER_OPEN;
    }
  }

  if (!this->is_at_position_target_() || (this->supports_tilt_() && !this->is_at_tilt_target_())) {
    // target not yet reached check for a direction change and recompute position
    CoverOperation dir = evaluate_direction_();
    if (dir != this->current_operation) {
      if (this->current_operation != COVER_OPERATION_IDLE) this->stop_direction_();
      this->start_direction_(dir);
    }
    else {
      this->recompute_position_();
    }
    this->last_recompute_time_ = now;
    if (now - this->last_publish_time_ > this->publish_interval_) {
      this->publish_state(false);
      this->last_publish_time_ = now;
    }
  }

  else if (this->current_operation != COVER_OPERATION_IDLE) {
    // target position reached
    this->position = this->target_position_;
    this->tilt = this->target_tilt_;
    if (this->has_built_in_endstop_ &&
        (((this->target_position_ == COVER_OPEN) && (!this->supports_tilt_() || this->target_tilt_ == COVER_OPEN)) ||
         ((this->target_position_ == COVER_CLOSED) && (!this->supports_tilt_() || this->target_tilt_ == COVER_CLOSED)))) {
      // Don't trigger stop, let the cover stop by itself, use an endstop sensor to stop the
      // motor except it is a stop command
      this->current_operation = COVER_OPERATION_IDLE;
    }
    else {
      // target reached, stop movement
      this->stop_direction_();
    }
    this->publish_state();
  }
}

cover::CoverOperation TimeBasedCover::evaluate_direction_() {
  CoverOperation dir = COVER_OPERATION_IDLE;

  if (this->is_at_position_target_()) {
    if (this->supports_tilt_() && !this->is_at_tilt_target_()) {
      // only tilt movement required
      dir = this->tilt > this->target_tilt_ ? COVER_OPERATION_CLOSING : COVER_OPERATION_OPENING;
    }
  }
  else {
    dir = this->target_position_ < this->position ? COVER_OPERATION_CLOSING : COVER_OPERATION_OPENING;
  }
  return dir;
}

float TimeBasedCover::get_setup_priority() const {
  return setup_priority::DATA;
}

CoverTraits TimeBasedCover::get_traits() {
  auto traits = CoverTraits();
  traits.set_supports_position(true);
  traits.set_supports_tilt(this->supports_tilt_());
  traits.set_is_assumed_state(this->assumed_state_);
  return traits;
}

void TimeBasedCover::control(const CoverCall &call) {
  ESP_LOGD(TAG, "got call at POS:%.1fs TILT:%.1fs", this->position, this->tilt);
  if (call.get_stop()) {
    ESP_LOGD(TAG, "  got STOP call");
    // let it stop with the next loops which checks for target position and tilt
    this->target_position_ = this->position;
    this->target_tilt_ = this->tilt;
    this->perform_stop_ = true;
  }
  else if (call.get_position().has_value()) {
    ESP_LOGD(TAG, "  got POS call to %.1fs", *call.get_position());
    this->target_position_ = *call.get_position();
    update_tilt_target_(call);
  }
  else if (this->supports_tilt_()) {
    ESP_LOGD(TAG, "  got TILT call to %.1fs", *call.get_tilt());
    this->target_position_ = this->position;
    update_tilt_target_(call);
  }
}

void TimeBasedCover::update_tilt_target_(const CoverCall &call) {
  if (this->supports_tilt_()) {
    if (!call.get_tilt().has_value()) {
      if ((this->target_position_ == this->position) && (this->position != COVER_CLOSED)) {
        // no position change and no tilt specified and not at the bottom
        this->target_tilt_ = this->tilt;
      }
      else {
        // no tilt specified, use 100% if the target is up or 0% if the target is down
        this->target_tilt_ = (this->target_position_ <= this->position) ? COVER_CLOSED : COVER_OPEN;
      }
    }
    else {
      this->target_tilt_ = *call.get_tilt();
    }
  }
}

void TimeBasedCover::stop_prev_trigger_() {
  if (this->prev_command_trigger_ != nullptr) {
    this->prev_command_trigger_->stop();
    this->prev_command_trigger_ = nullptr;
  }
}

bool TimeBasedCover::is_at_position_target_() const {
  switch (this->current_operation) {
    case COVER_OPERATION_OPENING:
      return this->position >= this->target_position_ &&
          this->position <= (this->target_position_ + std::abs(this->position_delta_));
    case COVER_OPERATION_CLOSING:
      return this->position <= this->target_position_ &&
          this->position >= (this->target_position_ - std::abs(this->position_delta_));
    case COVER_OPERATION_IDLE:
      return this->position == this->target_position_;
    default:
      return true;
  }
}

bool TimeBasedCover::is_at_tilt_target_() const {
  switch (this->current_operation) {
    case COVER_OPERATION_OPENING:
      return !this->supports_tilt_() ||
          (this->tilt >= this->target_tilt_ && this->tilt <= (this->target_tilt_ + std::abs(this->tilt_delta_)));
    case COVER_OPERATION_CLOSING:
      return !this->supports_tilt_() ||
          (this->tilt <= this->target_tilt_ && this->tilt >= (this->target_tilt_ - std::abs(this->tilt_delta_)));
    case COVER_OPERATION_IDLE:
      return !this->supports_tilt_() || this->tilt == this->target_tilt_;
    default:
      return true;
  }
}

void TimeBasedCover::start_direction_(CoverOperation dir) {
  const uint32_t now = millis();

  if (this->dir_change_delay_ > 0 && this->start_next_operation_time_ > now) {
    this->delayed_operation_ = dir;
    dir = COVER_OPERATION_IDLE;
  }
  Trigger<> *trig;
  switch (dir) {
    case COVER_OPERATION_OPENING:
      trig = this->open_trigger_;
      break;
    case COVER_OPERATION_CLOSING:
      trig = this->close_trigger_;
      break;
    default:
      return;
  }
  this->motor_on_time = now;
  this->current_operation = dir;
  this->stop_prev_trigger_();
  trig->trigger();
  this->prev_command_trigger_ = trig;
}

void TimeBasedCover::stop_direction_() {
  this->motor_on_time = 0;
  Trigger<> *trig = this->stop_trigger_;
  this->current_operation = COVER_OPERATION_IDLE;
  this->stop_prev_trigger_();
  trig->trigger();
  this->prev_command_trigger_ = trig;
}

void TimeBasedCover::recompute_position_() {
  float dir;
  float action_dur;

  switch (this->current_operation) {
  case COVER_OPERATION_OPENING:
    dir = 1.0f;
    action_dur = this->open_duration_;
    break;
  case COVER_OPERATION_CLOSING:
    dir = -1.0f;
    action_dur = this->close_duration_;
    break;
  default:
    return;
  }

  const uint32_t now = millis();
  if (this->supports_tilt_() &&
      ((this->current_operation == COVER_OPERATION_OPENING && this->tilt < COVER_OPEN) ||
       (this->current_operation == COVER_OPERATION_CLOSING && this->tilt > COVER_CLOSED))) {
    // first fully tilt up/down before moving up/down
    this->tilt_delta_ = dir * (now - this->last_recompute_time_) / this->tilt_duration_;
    this->tilt += this->tilt_delta_;
    this->tilt = clamp(this->tilt, COVER_CLOSED, COVER_OPEN);
  }
  else {
    this->position_delta_ = dir * (now - this->last_recompute_time_) / action_dur;
    this->position += this->position_delta_;
    this->position = clamp(this->position, COVER_CLOSED, COVER_OPEN);
  }
}

}  // namespace time_based
}  // namespace esphome
