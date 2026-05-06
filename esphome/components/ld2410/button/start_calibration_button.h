#pragma once

#include "esphome/components/button/button.h"
#include "../ld2410.h"

namespace esphome::ld2410 {

class StartCalibrationButton : public button::Button, public Parented<LD2410Component> {
 public:
  StartCalibrationButton() = default;

 protected:
  void press_action() override;
};

}  // namespace esphome::ld2410
