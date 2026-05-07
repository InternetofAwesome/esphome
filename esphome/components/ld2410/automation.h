#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "ld2410.h"

namespace esphome::ld2410 {

template<typename... Ts> class BluetoothPasswordSetAction final : public Action<Ts...> {
 public:
  explicit BluetoothPasswordSetAction(LD2410Component *ld2410_comp) : ld2410_comp_(ld2410_comp) {}
  TEMPLATABLE_VALUE(std::string, password)

  void play(const Ts &...x) override { this->ld2410_comp_->set_bluetooth_password(this->password_.value(x...)); }

 protected:
  LD2410Component *ld2410_comp_;
};

// Triggers auto-calibration with the specified mode, delay, and sample duration.
// If mode/delay/sample are omitted, the component's current values are used
// (i.e. whatever is set via the select/number entities or their defaults).
template<typename... Ts> class StartCalibrationAction : public Action<Ts...> {
 public:
  explicit StartCalibrationAction(LD2410Component *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(uint8_t, mode)
  TEMPLATABLE_VALUE(uint8_t, delay_s)
  TEMPLATABLE_VALUE(uint8_t, sample_s)

  void play(const Ts &...x) override {
    auto mode = this->mode_.has_value() ? static_cast<CalibrationMode>(this->mode_.value(x...))
                                        : this->parent_->get_calibration_mode();
    auto delay_s = this->delay_s_.has_value() ? this->delay_s_.value(x...) : this->parent_->get_calibration_delay_s();
    auto sample_s =
        this->sample_s_.has_value() ? this->sample_s_.value(x...) : this->parent_->get_calibration_sample_s();
    this->parent_->start_calibration(mode, delay_s, sample_s);
  }

 protected:
  LD2410Component *parent_;
};

template<typename... Ts> class ApplyCalibrationAction : public Action<Ts...> {
 public:
  explicit ApplyCalibrationAction(LD2410Component *parent) : parent_(parent) {}
  void play(const Ts &...x) override { this->parent_->apply_calibration(); }

 protected:
  LD2410Component *parent_;
};

template<typename... Ts> class DiscardCalibrationAction : public Action<Ts...> {
 public:
  explicit DiscardCalibrationAction(LD2410Component *parent) : parent_(parent) {}
  void play(const Ts &...x) override { this->parent_->discard_calibration(); }

 protected:
  LD2410Component *parent_;
};

}  // namespace esphome::ld2410
