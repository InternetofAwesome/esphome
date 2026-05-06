#pragma once

#include "esphome/components/number/number.h"
#include "../ld2410.h"

namespace esphome::ld2410 {

class CalibrationSampleNumber : public number::Number, public Parented<LD2410Component> {
 public:
  CalibrationSampleNumber() = default;

 protected:
  void control(float value) override;
};

}  // namespace esphome::ld2410
