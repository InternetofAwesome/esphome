#include "discard_calibration_button.h"

namespace esphome::ld2410 {

void DiscardCalibrationButton::press_action() { this->parent_->discard_calibration(); }

}  // namespace esphome::ld2410
