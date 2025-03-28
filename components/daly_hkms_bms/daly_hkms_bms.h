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
  SUB_SENSOR(cell_overvoltage_alarm_level)
  SUB_SENSOR(cell_undervoltage_alarm_level)
  SUB_SENSOR(cell_voltage_diff_alarm_level)
  SUB_SENSOR(overvoltage_alarm_level)
  SUB_SENSOR(undervoltage_alarm_level)
  SUB_SENSOR(charge_overcurrent_alarm_level)
  SUB_SENSOR(discharge_overcurrent_alarm_level)
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
#endif

#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(charging_mos_enabled)
  SUB_BINARY_SENSOR(discharging_mos_enabled)
  SUB_BINARY_SENSOR(precharging_mos_enabled)
  SUB_BINARY_SENSOR(balancing_active)
  SUB_BINARY_SENSOR(has_errors)
#endif

 protected:
  uint8_t daly_address_;
  uint32_t update_interval_fast_;

#ifdef USE_SENSOR
  sensor::Sensor *cell_voltage_sensors_[DALY_MODBUS_MAX_CELL_COUNT]{};
#endif
  std::vector<DalyHkmsBmsInput*> registered_inputs_{};
  uint16_t cell_voltage_sensors_max_{0};

  DalyHkmsCommandQueue *command_queue_;
};



}  // namespace daly_hkms_bms
}  // namespace esphome
