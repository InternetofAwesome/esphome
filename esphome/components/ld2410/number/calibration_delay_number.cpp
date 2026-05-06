#include "calibration_delay_number.h"

namespace esphome::ld2410 {

void CalibrationDelayNumber::control(float value) {
  this->publish_state(value);
  this->parent_->set_calibration_delay_s(static_cast<uint8_t>(value));
}

}  // namespace esphome::ld2410
