#include "calibration_sample_number.h"

namespace esphome::ld2410 {

void CalibrationSampleNumber::control(float value) {
  this->publish_state(value);
  this->parent_->set_calibration_sample_s(static_cast<uint8_t>(value));
}

}  // namespace esphome::ld2410
