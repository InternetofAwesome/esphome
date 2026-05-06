#include "apply_calibration_button.h"

namespace esphome::ld2410 {

void ApplyCalibrationButton::press_action() { this->parent_->apply_calibration(); }

}  // namespace esphome::ld2410
