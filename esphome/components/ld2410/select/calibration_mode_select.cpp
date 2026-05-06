#include "calibration_mode_select.h"

namespace esphome::ld2410 {

void CalibrationModeSelect::control(size_t index) {
  this->publish_state(index);
  // Changing mode alone doesn't start calibration — user must press Start button.
  // Store the selection; start_calibration() reads cal_mode_ when triggered.
  this->parent_->set_calibration_mode(static_cast<CalibrationMode>(index));
}

}  // namespace esphome::ld2410
