#pragma once

#include "esphome/components/select/select.h"
#include "../ld2410.h"

namespace esphome::ld2410 {

class CalibrationModeSelect : public select::Select, public Parented<LD2410Component> {
 public:
  CalibrationModeSelect() = default;

 protected:
  void control(size_t index) override;
};

}  // namespace esphome::ld2410
