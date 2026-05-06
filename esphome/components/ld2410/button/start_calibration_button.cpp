#include "start_calibration_button.h"

namespace esphome::ld2410 {

void StartCalibrationButton::press_action() {
  this->parent_->start_calibration(
      this->parent_->get_calibration_mode(),
      this->parent_->get_calibration_delay_s(),
      this->parent_->get_calibration_sample_s());
}

}  // namespace esphome::ld2410
