#pragma once
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#ifdef USE_SELECT
#include "esphome/components/select/select.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#include "esphome/components/ld24xx/ld24xx.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"
#include "esphome/core/helpers.h"

#include <array>

namespace esphome::ld2410 {

using namespace ld24xx;

// Engineering data frame is 45 bytes; +1 for null terminator, +4 so that a frame footer always
// lands inside the buffer during footer-based resynchronization after losing sync.
static constexpr uint8_t MAX_LINE_LENGTH = 50;
static constexpr uint8_t TOTAL_GATES = 9;  // Total number of gates supported by the LD2410

enum class CalibrationMode : uint8_t {
  OFF = 0,
  AVERAGE = 1,
  MAXIMUM = 2,
  INTELLIGENT = 3,
};

enum class CalibrationState : uint8_t {
  IDLE = 0,
  DELAY = 1,      // countdown before sampling begins
  SAMPLING = 2,   // collecting energy data (Average/Maximum)
  READY = 3,      // thresholds computed, awaiting apply/discard
  APPLYING = 4,   // writing thresholds to device
  // Intelligent-mode firmware states
  FW_WAITING = 5,   // firmware sampling in progress
  FW_SUCCESS = 6,
  FW_FAILED = 7,
};

class LD2410Component final : public Component, public uart::UARTDevice {
#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(out_pin_presence_status)
  SUB_BINARY_SENSOR(moving_target)
  SUB_BINARY_SENSOR(still_target)
  SUB_BINARY_SENSOR(target)
#endif
#ifdef USE_SENSOR
  SUB_SENSOR_WITH_DEDUP(light, uint8_t)
  SUB_SENSOR_WITH_DEDUP(detection_distance, int)
  SUB_SENSOR_WITH_DEDUP(moving_target_distance, int)
  SUB_SENSOR_WITH_DEDUP(moving_target_energy, uint8_t)
  SUB_SENSOR_WITH_DEDUP(still_target_distance, int)
  SUB_SENSOR_WITH_DEDUP(still_target_energy, uint8_t)
#endif
#ifdef USE_TEXT_SENSOR
  SUB_TEXT_SENSOR(version)
  SUB_TEXT_SENSOR(mac)
  SUB_TEXT_SENSOR(calibration_status)
#endif
#ifdef USE_NUMBER
  SUB_NUMBER(light_threshold)
  SUB_NUMBER(max_move_distance_gate)
  SUB_NUMBER(max_still_distance_gate)
  SUB_NUMBER(timeout)
  SUB_NUMBER(calibration_delay)
  SUB_NUMBER(calibration_sample)
#endif
#ifdef USE_SELECT
  SUB_SELECT(baud_rate)
  SUB_SELECT(distance_resolution)
  SUB_SELECT(light_function)
  SUB_SELECT(out_pin_level)
  SUB_SELECT(calibration_mode)
#endif
#ifdef USE_SWITCH
  SUB_SWITCH(bluetooth)
  SUB_SWITCH(engineering_mode)
#endif
#ifdef USE_BUTTON
  SUB_BUTTON(factory_reset)
  SUB_BUTTON(query)
  SUB_BUTTON(restart)
  SUB_BUTTON(start_calibration)
  SUB_BUTTON(apply_calibration)
  SUB_BUTTON(discard_calibration)
#endif

 public:
  void setup() override;
  void dump_config() override;
  void loop() override;
  void set_light_out_control();
#ifdef USE_NUMBER
  void set_gate_still_threshold_number(uint8_t gate, number::Number *n);
  void set_gate_move_threshold_number(uint8_t gate, number::Number *n);
  void set_max_distances_timeout();
  void set_gate_threshold(uint8_t gate);
#endif
#ifdef USE_SENSOR
  void set_gate_move_sensor(uint8_t gate, sensor::Sensor *s);
  void set_gate_still_sensor(uint8_t gate, sensor::Sensor *s);
#endif
  void set_bluetooth_password(const std::string &password);
  void set_engineering_mode(bool enable);
  void read_all_info();
  void restart_and_read_all_info();
  void set_bluetooth(bool enable);
  void set_distance_resolution(const char *state);
  void set_baud_rate(const char *state);
  void factory_reset();

  // Calibration API — entities and actions call these.
  void start_calibration(CalibrationMode mode, uint8_t delay_s, uint8_t sample_s);
  void apply_calibration();
  void discard_calibration();
  void set_calibration_mode(CalibrationMode mode) { this->cal_mode_ = mode; }
  CalibrationMode get_calibration_mode() const { return this->cal_mode_; }
  void set_calibration_delay_s(uint8_t s) { this->cal_delay_s_ = s; }
  uint8_t get_calibration_delay_s() const { return this->cal_delay_s_; }
  void set_calibration_sample_s(uint8_t s) { this->cal_sample_s_ = s; }
  uint8_t get_calibration_sample_s() const { return this->cal_sample_s_; }
  CalibrationState get_calibration_state() const { return this->cal_state_; }
  // Returns human-readable status for a text_sensor (e.g. "delay 7/10s", "sampling 23/60s").
  std::string get_calibration_status_str() const;

 protected:
  void send_command_(uint8_t command_str, const uint8_t *command_value, uint8_t command_value_len);
  void set_config_mode_(bool enable);
  void handle_periodic_data_();
  bool handle_ack_data_();
  void readline_(int readch);
  void query_parameters_();
  void get_version_();
  void get_mac_();
  void get_distance_resolution_();
  void query_light_control_();
  void restart_();

  void tick_calibration_();
  void compute_thresholds_();
  void apply_thresholds_();

  uint8_t light_function_ = 0;
  uint8_t light_threshold_ = 0;
  uint8_t out_pin_level_ = 0;
  uint8_t buffer_pos_ = 0;  // where to resume processing/populating buffer
  uint8_t buffer_data_[MAX_LINE_LENGTH];
  uint8_t mac_address_[6] = {0, 0, 0, 0, 0, 0};
  uint8_t version_[6] = {0, 0, 0, 0, 0, 0};
  bool bluetooth_on_{false};
#ifdef USE_NUMBER
  std::array<number::Number *, TOTAL_GATES> gate_move_threshold_numbers_{};
  std::array<number::Number *, TOTAL_GATES> gate_still_threshold_numbers_{};
#endif
#ifdef USE_SENSOR
  std::array<SensorWithDedup<uint8_t>, TOTAL_GATES> gate_move_sensors_{};
  std::array<SensorWithDedup<uint8_t>, TOTAL_GATES> gate_still_sensors_{};
#endif

  // Latest per-gate energies captured from engineering-mode frames.
  // Populated whenever engineering mode is active, regardless of whether the
  // user has configured gate energy sensors — calibration needs them without
  // requiring sensor entities to be declared.
  std::array<uint8_t, TOTAL_GATES> latest_move_energy_{};
  std::array<uint8_t, TOTAL_GATES> latest_still_energy_{};

  // Calibration state machine
  CalibrationState cal_state_{CalibrationState::IDLE};
  CalibrationMode cal_mode_{CalibrationMode::OFF};
  uint8_t cal_delay_s_{10};
  uint8_t cal_sample_s_{60};
  uint32_t cal_phase_start_ms_{0};
  uint32_t cal_last_poll_ms_{0};
  uint16_t cal_samples_collected_{0};

  // Accumulators for Average/Maximum: [gate][move/still], indexed 0=move 1=still
  std::array<uint32_t, TOTAL_GATES> cal_move_accum_{};
  std::array<uint32_t, TOTAL_GATES> cal_still_accum_{};
  std::array<uint8_t, TOTAL_GATES> cal_move_max_{};
  std::array<uint8_t, TOTAL_GATES> cal_still_max_{};

  // Computed thresholds ready to apply
  std::array<uint8_t, TOTAL_GATES> cal_move_result_{};
  std::array<uint8_t, TOTAL_GATES> cal_still_result_{};
};

}  // namespace esphome::ld2410
