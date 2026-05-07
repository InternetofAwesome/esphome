#include "calibration_delay_number.h"

namespace esphome::ld2410 {

static const uint8_t DEFAULT_DELAY_S = 10;

void CalibrationDelayNumber::setup() {
  this->pref_ = this->make_entity_preference<uint8_t>();
  uint8_t value;
  if (!this->pref_.load(&value))
    value = DEFAULT_DELAY_S;
  this->parent_->set_calibration_delay_s(value);
  this->publish_state(value);
}

void CalibrationDelayNumber::control(float value) {
  auto v = static_cast<uint8_t>(value);
  this->pref_.save(&v);
  this->publish_state(value);
  this->parent_->set_calibration_delay_s(v);
}

void CalibrationDelayNumber::reset_preference() {
  uint8_t value = DEFAULT_DELAY_S;
  this->pref_.save(&value);
  this->parent_->set_calibration_delay_s(value);
  this->publish_state(value);
}

}  // namespace esphome::ld2410
