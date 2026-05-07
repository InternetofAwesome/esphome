#pragma once

#include "esphome/components/number/number.h"
#include "esphome/core/preferences.h"
#include "../ld2410.h"

namespace esphome::ld2410 {

class CalibrationSampleNumber : public number::Number, public Parented<LD2410Component> {
 public:
  CalibrationSampleNumber() = default;
  void setup() override;
  void reset_preference();

 protected:
  void control(float value) override;
  ESPPreferenceObject pref_;
};

}  // namespace esphome::ld2410
