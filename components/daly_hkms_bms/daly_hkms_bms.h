#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#include "esphome/components/modbus/modbus.h"
#include "daly_hkms_bms_queue.h"

#include <vector>

namespace esphome {
namespace daly_hkms_bms {

static const uint8_t DALY_MODBUS_MAX_CELL_COUNT = 48;

class DalyHkmsBmsInput {
  public:
    DalyHkmsBmsInput() {}
    virtual uint16_t get_reg_addr() = 0;
    virtual void handle_update(uint16_t value) = 0;
 };

  struct DalyHkmsStatus
  {
    // Code 0-1
    uint8_t lvl_cell_ovp : 3;
    uint8_t lvl_cell_uvp : 3;
    bool smart_charger_connected : 1;
    bool err_smart_charger_connection : 1;

    uint8_t lvl_cell_volt_diff : 3;
    uint8_t lvl_chg_overtemp : 3;
    bool smart_discharger_connected : 1;
    bool err_smart_discharger_connection : 1;

    // Code 2-3
    uint8_t lvl_chg_undertemp : 3;
    uint8_t lvl_dschg_overtemp : 3;
    bool err_chg_mos_temp_high : 1;
    bool err_chg_mos_temp_detect : 1;

    uint8_t lvl_dschg_undertemp : 3;
    uint8_t lvl_temp_diff : 3;
    bool err_dschg_mos_temp_high : 1;
    bool err_dschg_mos_temp_detect : 1;

    // Code 4-5
    uint8_t lvl_total_ovp : 3;
    uint8_t lvl_total_uvp : 3;
    bool err_short_circuit : 1;
    bool upgrade_sign: 1;

    uint8_t lvl_chg_ocp : 3;
    uint8_t lvl_dschg_ocp : 3;
    bool err_chg_undervoltage : 1;
    bool err_dschg_overvoltage : 1;

    // Code 6-7
    uint8_t lvl_soc_low : 3;
    uint8_t lvl_soh_low : 3;
    bool parallel_comm : 1;
    bool err_parallel_comm: 1;

    uint8_t lvl_mos_overtemp : 3;
    uint8_t lvl_thermal_runaway : 3;
    bool : 1;
    bool : 1;

    // Code 8-9
    uint16_t : 16;

    // Code 10-11
    uint8_t : 8;

    bool err_afe_chip: 1;
    bool err_afe_comm: 1;
    bool err_afe_sampling: 1;
    bool err_volt_detect: 1;
    bool err_volt_detect_disconnected: 1;
    bool err_volt_total_detect: 1;
    bool err_curr_detect: 1;
    bool err_temp_detect: 1;

    // Code 12-13
    bool err_temp_disconnected: 1;
    bool err_eeprom: 1;
    bool err_flash: 1;
    bool err_rtc: 1;
    bool err_chg_mos: 1;
    bool err_dschg_mos: 1;
    bool err_prechg_mos: 1;
    bool err_prechg: 1;

    bool chg_mos_off_bus: 1;
    bool dschg_mos_off_bus: 1;
    bool chg_mos_off_switch: 1;
    bool dschg_mos_off_switch: 1;
    bool fan_active: 1;
    bool heating_active: 1;
    bool current_limit_active: 1;
    bool err_heating: 1;
  };

  static_assert(sizeof(DalyHkmsStatus) == 14);
 

class DalyHkmsBmsComponent : public PollingComponent, public modbus::ModbusDevice {
 public:
  void setup() override;
  void loop() override;
  void update() override;
  void update_fast();
  void on_modbus_data(const std::vector<uint8_t> &data) override;
  void dump_config() override;

  void set_daly_address(uint8_t address);

  void set_update_interval_fast(uint32_t update_interval_fast) {
    update_interval_fast_ = update_interval_fast;
  }

  void write_register(uint16_t reg, uint16_t value);

  void register_input(DalyHkmsBmsInput *input) {
    this->registered_inputs_.push_back(input);
  }

#ifdef USE_SENSOR
  void set_cell_voltage_sensor(uint16_t cell, sensor::Sensor *sensor) {
    if (cell > this->cell_voltage_sensors_max_)
      this->cell_voltage_sensors_max_ = cell;
    this->cell_voltage_sensors_[cell - 1] = sensor;
  };

  SUB_SENSOR(voltage)
  SUB_SENSOR(current)
  SUB_SENSOR(battery_level)
  SUB_SENSOR(max_cell_voltage)
  SUB_SENSOR(max_cell_voltage_number)
  SUB_SENSOR(min_cell_voltage)
  SUB_SENSOR(min_cell_voltage_number)
  SUB_SENSOR(delta_cell_voltage)
  SUB_SENSOR(max_temperature)
  SUB_SENSOR(max_temperature_probe_number)
  SUB_SENSOR(min_temperature)
  SUB_SENSOR(min_temperature_probe_number)
  SUB_SENSOR(remaining_capacity)
  SUB_SENSOR(cycles)
  SUB_SENSOR(balance_current)
  SUB_SENSOR(cells_number)
  SUB_SENSOR(temps_number)
  SUB_SENSOR(power)
  SUB_SENSOR(charge_power)
  SUB_SENSOR(discharge_power)
  SUB_SENSOR(energy)
  SUB_SENSOR(temperature_mos)
  SUB_SENSOR(temperature_board)

  SUB_SENSOR(alarm_level_cell_overvoltage)
  SUB_SENSOR(alarm_level_cell_undervoltage)
  SUB_SENSOR(alarm_level_cell_voltage_diff)
  SUB_SENSOR(alarm_level_charge_overtemperature)
  SUB_SENSOR(alarm_level_charge_undertemperature)
  SUB_SENSOR(alarm_level_discharge_overtemperature)
  SUB_SENSOR(alarm_level_discharge_undertemperature)
  SUB_SENSOR(alarm_level_temperature_diff)
  SUB_SENSOR(alarm_level_overvoltage)
  SUB_SENSOR(alarm_level_undervoltage)
  SUB_SENSOR(alarm_level_charge_overcurrent)
  SUB_SENSOR(alarm_level_discharge_overcurrent)
  SUB_SENSOR(alarm_level_soc_low)
  SUB_SENSOR(alarm_level_soh_low)
  SUB_SENSOR(alarm_level_mos_overtemperature)

  SUB_SENSOR(temperature_1)
  SUB_SENSOR(temperature_2)
  SUB_SENSOR(temperature_3)
  SUB_SENSOR(temperature_4)
  SUB_SENSOR(temperature_5)
  SUB_SENSOR(temperature_6)
  SUB_SENSOR(temperature_7)
  SUB_SENSOR(temperature_8)
#endif

#ifdef USE_TEXT_SENSOR
  SUB_TEXT_SENSOR(status)
  SUB_TEXT_SENSOR(alerts)
#endif

#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(charging_mos_enabled)
  SUB_BINARY_SENSOR(discharging_mos_enabled)
  SUB_BINARY_SENSOR(precharging_mos_enabled)
  SUB_BINARY_SENSOR(balancing_active)

  SUB_BINARY_SENSOR(error_charge_mos_overtemperature)
  SUB_BINARY_SENSOR(error_charge_mos_temperature_detect)
  SUB_BINARY_SENSOR(error_discharge_mos_overtemperature)
  SUB_BINARY_SENSOR(error_discharge_mos_temperature_detect)
  SUB_BINARY_SENSOR(error_short_circuit)
  SUB_BINARY_SENSOR(has_warnings)
  SUB_BINARY_SENSOR(has_errors)

  void set_cell_balancing_sensor(uint16_t cell, binary_sensor::BinarySensor *sensor) {
    if (cell > this->cell_balancing_sensors_max_)
      this->cell_balancing_sensors_max_ = cell;
    this->cell_balancing_sensors_[cell - 1] = sensor;
  };
#endif

 protected:
  uint8_t daly_address_;
  uint32_t update_interval_fast_;

#ifdef USE_SENSOR
  sensor::Sensor *cell_voltage_sensors_[DALY_MODBUS_MAX_CELL_COUNT]{};
#endif
  uint16_t cell_voltage_sensors_max_{0};

#ifdef USE_BINARY_SENSOR
  binary_sensor::BinarySensor *cell_balancing_sensors_[DALY_MODBUS_MAX_CELL_COUNT]{};
#endif
  uint16_t cell_balancing_sensors_max_{0};

  std::vector<DalyHkmsBmsInput*> registered_inputs_{};
  DalyHkmsCommandQueue *command_queue_;
};



}  // namespace daly_hkms_bms
}  // namespace esphome
