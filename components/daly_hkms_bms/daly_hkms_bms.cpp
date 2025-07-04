#include "daly_hkms_bms.h"
#include "daly_hkms_bms_registers.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

#include <cinttypes>
#include <cstring>
#include <sstream>

namespace esphome {
namespace daly_hkms_bms {

static const char *const TAG = "daly_hkms_bms";

// the DALY BMS only _kinda_ does Modbus. The device address is offset by 0x80 (so BMS #1 has address 0x81)
// which would be fine, however the Modbus address of the response has a different offset of 0x50,
// which makes this very much non-standard-compliant...
static const uint8_t DALY_MODBUS_REQUEST_ADDRESS_OFFSET = 0x80;
static const uint8_t DALY_MODBUS_RESPONSE_ADDRESS_OFFSET = 0x50;

static const uint8_t MODBUS_CMD_READ_HOLDING_REGISTERS = 0x03;
static const uint8_t MODBUS_CMD_WRITE_SINGLE_REGISTER = 0x06;

void DalyHkmsBmsComponent::set_daly_address(uint8_t daly_address) {
  this->daly_address_ = daly_address;

  // set ModbusDevice address to the response address so the modbus component forwards
  // the response of this device to this component
  uint8_t modbus_response_address = daly_address + DALY_MODBUS_RESPONSE_ADDRESS_OFFSET;
  this->set_address(modbus_response_address);
}

void DalyHkmsBmsComponent::setup() {
  command_queue_ = DalyHkmsCommandQueue::get_for_modbus(this->parent_);

  if (this->update_interval_fast_ > 0)
    this->set_interval("update_fast", this->update_interval_fast_, [this]() { this->update_fast(); });
}

void DalyHkmsBmsComponent::loop() {
  // The bus might be slow, or there might be other devices, or other components might be talking to our device.
  if (this->waiting_for_response()) {
    return;
  }

  QueueItem to_send;
  if (!command_queue_->try_get_to_send(daly_address_, &to_send)) {
    return;
  }

  // send the request using Modbus directly instead of ModbusDevice so we can send the data with the request address
  uint8_t modbus_device_request_address = this->daly_address_ + DALY_MODBUS_REQUEST_ADDRESS_OFFSET;

  switch (to_send.cmd) {
    case MODBUS_CMD_READ_HOLDING_REGISTERS:
    {
      ESP_LOGV(TAG, "Sending modbus read request to %d: start register %" PRIu16 ", register count %" PRIu16, this->daly_address_,
        to_send.register_address, to_send.data);
      this->parent_->send(modbus_device_request_address, MODBUS_CMD_READ_HOLDING_REGISTERS, to_send.register_address, to_send.data,
        0, nullptr);
      break;
    }
    case MODBUS_CMD_WRITE_SINGLE_REGISTER:
    {
      ESP_LOGV(TAG, "Sending modbus write request to %d: register %" PRIu16 ", value %" PRIu16, this->daly_address_,
        to_send.register_address, to_send.data);
      
        uint8_t reg_value[2] = { uint8_t((to_send.data >> 8) & 0xFF), uint8_t(to_send.data & 0xFF) };
      this->parent_->send(modbus_device_request_address, MODBUS_CMD_WRITE_SINGLE_REGISTER, to_send.register_address, 1,
        2, reg_value);
      break;
    }
  
    default:
      ESP_LOGE(TAG, "Invalid command %d", to_send.cmd);
      return;
  }
}

void DalyHkmsBmsComponent::write_register(uint16_t reg, uint16_t value) {
  this->command_queue_->add_or_update(true, 
    {
      .daly_address = this->daly_address_,
      .cmd = MODBUS_CMD_WRITE_SINGLE_REGISTER,
      .register_address = reg,
      .data = value
    });
}

void DalyHkmsBmsComponent::update() {
  if (this->cell_voltage_sensors_max_ > 0) {
    this->command_queue_->add_or_update(false, 
      {
        .daly_address = this->daly_address_,
        .cmd = MODBUS_CMD_READ_HOLDING_REGISTERS,
        .register_address = DALY_MODBUS_ADDR_CELL_VOLT_1,
        .data = this->cell_voltage_sensors_max_ // avoid reading all 48 cell voltages if we only want 16 or so
      });
  }

  this->command_queue_->add_or_update(false, 
    {
      .daly_address = this->daly_address_,
      .cmd = MODBUS_CMD_READ_HOLDING_REGISTERS,
      .register_address = DALY_MODBUS_ADDR_CELL_TEMP_1,
      .data = DALY_MODBUS_ADDR_CELL_TEMP_DIFF - DALY_MODBUS_ADDR_CELL_TEMP_1 + 1
    });
  this->command_queue_->add_or_update(false, 
    {
      .daly_address = this->daly_address_,
      .cmd = MODBUS_CMD_READ_HOLDING_REGISTERS,
      .register_address = DALY_MODBUS_ADDR_CHG_DSCHG_STATUS,
      .data = DALY_MODBUS_ADDR_HEATING_TEMP - DALY_MODBUS_ADDR_CHG_DSCHG_STATUS + 1
    });
  this->command_queue_->add_or_update(false, 
    {
      .daly_address = this->daly_address_,
      .cmd = MODBUS_CMD_READ_HOLDING_REGISTERS,
      .register_address = DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_00_01,
      .data = DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_12_13 - DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_00_01 + 1
    });
  this->command_queue_->add_or_update(false, 
    {
      .daly_address = this->daly_address_,
      .cmd = MODBUS_CMD_READ_HOLDING_REGISTERS,
      .register_address = DALY_MODBUS_ADDR_CHG_MOS_CONTROL,
      .data = DALY_MODBUS_ADDR_DSCHG_MOS_CONTROL - DALY_MODBUS_ADDR_CHG_MOS_CONTROL + 1
    });
}

void DalyHkmsBmsComponent::update_fast() {
  this->command_queue_->add_or_update(true, 
    {
      .daly_address = this->daly_address_,
      .cmd = MODBUS_CMD_READ_HOLDING_REGISTERS,
      .register_address = DALY_MODBUS_ADDR_VOLT,
      .data = DALY_MODBUS_ADDR_CURR - DALY_MODBUS_ADDR_VOLT + 1
    });
}

void DalyHkmsBmsComponent::on_modbus_data(const std::vector<uint8_t> &data) {
  // Other components might be sending commands to our device. But we don't get called with enough
  // context to know what is what. So if we didn't do a send, we ignore the data.
  QueueItem request;
  if (!this->command_queue_->pop_pending(this->daly_address_, &request)) {
    ESP_LOGD(TAG, "#%d Got data without requesting it first", this->daly_address_);
    return;
  }

  ESP_LOGV(TAG, "#%d Got modbus response: %d bytes", this->daly_address_, data.size());

  uint16_t register_offset;
  uint16_t register_count;

  switch (request.cmd) {
    case MODBUS_CMD_READ_HOLDING_REGISTERS:
      register_offset = request.register_address;
      register_count = request.data;
      break;
    case MODBUS_CMD_WRITE_SINGLE_REGISTER:
      register_offset = request.register_address;
      register_count = 1;
      break;
    default:
      ESP_LOGE(TAG, "Invalid command %d", request.cmd);
      return;
  }

  if (data.size() < register_count * 2) {
    ESP_LOGD(TAG, "Not enough data in modbus response");
    return;
  }

  auto has_register = [&](uint16_t i) -> bool {
    return i >= register_offset && i < (register_offset + register_count);
  };
  auto get_register = [&](uint16_t i) -> uint16_t {
    return encode_uint16(data[(i - register_offset) * 2], data[(i - register_offset) * 2 + 1]);
  };

  bool has_status = false;
  DalyHkmsStatus status = {};

  if (has_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_00_01) && has_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_12_13)) {
    uint16_t registers[] = {
      convert_little_endian(get_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_00_01)),
      convert_little_endian(get_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_02_03)),
      convert_little_endian(get_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_04_05)),
      convert_little_endian(get_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_06_07)),
      convert_little_endian(get_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_08_09)),
      convert_little_endian(get_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_10_11)),
      convert_little_endian(get_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_12_13))
    };
    static_assert(sizeof(registers) == sizeof(DalyHkmsStatus));

    std::memcpy(&status, registers, sizeof(DalyHkmsStatus));
    has_status = true;
  }

#ifdef USE_SENSOR
  auto publish_sensor_state_raw = [](sensor::Sensor *sensor, float value) -> void {
    if (sensor == nullptr)
      return;
    sensor->publish_state(value);
  };

  auto publish_sensor_state = [&](sensor::Sensor *sensor, uint16_t i, int16_t offset, float factor,
    int32_t unavailable_value = -1) -> void {
    if (sensor == nullptr || !has_register(i))
      return;
    uint16_t register_value = get_register(i);
    float value = register_value == unavailable_value ? NAN : (register_value + offset) * factor;
    sensor->publish_state(value);
  };

  for (uint16_t i = 0; i < this->cell_voltage_sensors_max_; i++) {
    publish_sensor_state(this->cell_voltage_sensors_[i], DALY_MODBUS_ADDR_CELL_VOLT_1 + i, 0, 0.001);
  }
  publish_sensor_state(this->temperature_1_sensor_, DALY_MODBUS_ADDR_CELL_TEMP_1, -40, 1, 255);
  publish_sensor_state(this->temperature_2_sensor_, DALY_MODBUS_ADDR_CELL_TEMP_1 + 1, -40, 1, 255);
  publish_sensor_state(this->temperature_3_sensor_, DALY_MODBUS_ADDR_CELL_TEMP_1 + 2, -40, 1, 255);
  publish_sensor_state(this->temperature_4_sensor_, DALY_MODBUS_ADDR_CELL_TEMP_1 + 3, -40, 1, 255);
  publish_sensor_state(this->temperature_5_sensor_, DALY_MODBUS_ADDR_CELL_TEMP_1 + 4, -40, 1, 255);
  publish_sensor_state(this->temperature_6_sensor_, DALY_MODBUS_ADDR_CELL_TEMP_1 + 5, -40, 1, 255);
  publish_sensor_state(this->temperature_7_sensor_, DALY_MODBUS_ADDR_CELL_TEMP_1 + 6, -40, 1, 255);
  publish_sensor_state(this->temperature_8_sensor_, DALY_MODBUS_ADDR_CELL_TEMP_1 + 7, -40, 1, 255);

  publish_sensor_state(this->voltage_sensor_, DALY_MODBUS_ADDR_VOLT, 0, 0.1);
  publish_sensor_state(this->current_sensor_, DALY_MODBUS_ADDR_CURR, -30000, 0.1);
  publish_sensor_state(this->battery_level_sensor_, DALY_MODBUS_ADDR_SOC, 0, 0.1);

  publish_sensor_state(this->cells_number_sensor_, DALY_MODBUS_ADDR_CELL_COUNT, 0, 1);
  publish_sensor_state(this->temps_number_sensor_, DALY_MODBUS_ADDR_CELL_TEMP_COUNT, 0, 1);

  publish_sensor_state(this->max_cell_voltage_sensor_, DALY_MODBUS_ADDR_CELL_VOLT_MAX, 0, 0.001);
  publish_sensor_state(this->max_cell_voltage_number_sensor_, DALY_MODBUS_ADDR_CELL_VOLT_MAX_NUM, 0, 1);

  publish_sensor_state(this->min_cell_voltage_sensor_, DALY_MODBUS_ADDR_CELL_VOLT_MIN, 0, 0.001);
  publish_sensor_state(this->min_cell_voltage_number_sensor_, DALY_MODBUS_ADDR_CELL_VOLT_MIN_NUM, 0, 1);

  publish_sensor_state(this->delta_cell_voltage_sensor_, DALY_MODBUS_ADDR_CELL_VOLT_DIFF, 0, 0.001);

  publish_sensor_state(this->max_temperature_sensor_, DALY_MODBUS_ADDR_CELL_TEMP_MAX, -40, 1, 255);
  publish_sensor_state(this->max_temperature_probe_number_sensor_, DALY_MODBUS_ADDR_CELL_TEMP_MAX_NUM, 0, 1);

  publish_sensor_state(this->min_temperature_sensor_, DALY_MODBUS_ADDR_CELL_TEMP_MIN, -40, 1, 255);
  publish_sensor_state(this->min_temperature_probe_number_sensor_, DALY_MODBUS_ADDR_CELL_TEMP_MIN_NUM, 0, 1);

  publish_sensor_state(this->remaining_capacity_sensor_, DALY_MODBUS_ADDR_REMAINING_CAPACITY, 0, 0.1);
  publish_sensor_state(this->cycles_sensor_, DALY_MODBUS_ADDR_CYCLES, 0, 1);

  publish_sensor_state(this->balance_current_sensor_, DALY_MODBUS_ADDR_BALANCE_CURRENT, -30000, 0.001);

  if (has_register(DALY_MODBUS_ADDR_POWER) && has_register(DALY_MODBUS_ADDR_CHG_DSCHG_STATUS)) {
    uint16_t power = get_register(DALY_MODBUS_ADDR_POWER);
    uint16_t chg_dschg_status = get_register(DALY_MODBUS_ADDR_CHG_DSCHG_STATUS);
    switch (chg_dschg_status) {
      case 0: // stationary
        publish_sensor_state_raw(this->power_sensor_, 0);
        publish_sensor_state_raw(this->charge_power_sensor_, 0);
        publish_sensor_state_raw(this->discharge_power_sensor_, 0);
        break;

      case 1: // charging
        publish_sensor_state_raw(this->power_sensor_, power);
        publish_sensor_state_raw(this->charge_power_sensor_, power);
        publish_sensor_state_raw(this->discharge_power_sensor_, 0);
        break;

      case 2: // discharging
        publish_sensor_state_raw(this->power_sensor_, -power);
        publish_sensor_state_raw(this->charge_power_sensor_, 0);
        publish_sensor_state_raw(this->discharge_power_sensor_, power);
        break;
      
      default:
        break;
    }
  }

  publish_sensor_state(this->energy_sensor_, DALY_MODBUS_ADDR_ENERGY, 0, 1);

  publish_sensor_state(this->temperature_mos_sensor_, DALY_MODBUS_ADDR_MOS_TEMP, -40, 1, 255);
  publish_sensor_state(this->temperature_board_sensor_, DALY_MODBUS_ADDR_BOARD_TEMP, -40, 1, 255);

  if (has_status) {
    publish_sensor_state_raw(this->alarm_level_cell_overvoltage_sensor_, status.lvl_cell_ovp);
    publish_sensor_state_raw(this->alarm_level_cell_undervoltage_sensor_, status.lvl_cell_uvp);
    publish_sensor_state_raw(this->alarm_level_cell_voltage_diff_sensor_, status.lvl_cell_volt_diff);
    publish_sensor_state_raw(this->alarm_level_charge_overtemperature_sensor_, status.lvl_chg_overtemp);

    publish_sensor_state_raw(this->alarm_level_charge_undertemperature_sensor_, status.lvl_chg_undertemp);
    publish_sensor_state_raw(this->alarm_level_discharge_overtemperature_sensor_, status.lvl_dschg_overtemp);
    publish_sensor_state_raw(this->alarm_level_discharge_undertemperature_sensor_, status.lvl_dschg_undertemp);
    publish_sensor_state_raw(this->alarm_level_temperature_diff_sensor_, status.lvl_temp_diff);

    publish_sensor_state_raw(this->alarm_level_overvoltage_sensor_, status.lvl_total_ovp);
    publish_sensor_state_raw(this->alarm_level_undervoltage_sensor_, status.lvl_total_uvp);
    publish_sensor_state_raw(this->alarm_level_charge_overcurrent_sensor_, status.lvl_chg_ocp);
    publish_sensor_state_raw(this->alarm_level_discharge_overcurrent_sensor_, status.lvl_dschg_ocp);

    publish_sensor_state_raw(this->alarm_level_soc_low_sensor_, status.lvl_soc_low);
    publish_sensor_state_raw(this->alarm_level_soh_low_sensor_, status.lvl_soh_low);
    publish_sensor_state_raw(this->alarm_level_mos_overtemperature_sensor_, status.lvl_mos_overtemp);
  }

#endif

#ifdef USE_TEXT_SENSOR
  if (this->status_text_sensor_ != nullptr && has_register(DALY_MODBUS_ADDR_CHG_DSCHG_STATUS)) {
    switch (get_register(DALY_MODBUS_ADDR_CHG_DSCHG_STATUS)) {
      case 0:
        this->status_text_sensor_->publish_state("Stationary");
        break;
      case 1:
        this->status_text_sensor_->publish_state("Charging");
        break;
      case 2:
        this->status_text_sensor_->publish_state("Discharging");
        break;
      default:
        break;
    }
  }
#endif

#ifdef USE_BINARY_SENSOR
  auto publish_binary_sensor_state = [](binary_sensor::BinarySensor *sensor, bool value) -> void {
    if (sensor == nullptr)
      return;
    sensor->publish_state(value);
  };

  if (this->balancing_active_binary_sensor_ && has_register(DALY_MODBUS_ADDR_BALANCE_STATUS)) {
    this->balancing_active_binary_sensor_->publish_state(get_register(DALY_MODBUS_ADDR_BALANCE_STATUS) > 0);
  }

  if (has_register(DALY_MODBUS_ADDR_BALANCE_STATUS_PER_CELL_01_TO_16) && has_register(DALY_MODBUS_ADDR_BALANCE_STATUS_PER_CELL_33_TO_48)) {
    uint16_t reg_val = get_register(DALY_MODBUS_ADDR_BALANCE_STATUS_PER_CELL_01_TO_16);
    for (uint16_t i = 0; i < std::min<uint16_t>(this->cell_balancing_sensors_max_, 16); i++) {
      if (this->cell_balancing_sensors_[i] != nullptr)
        this->cell_balancing_sensors_[i]->publish_state(reg_val & (1 << i));
    }

    reg_val = get_register(DALY_MODBUS_ADDR_BALANCE_STATUS_PER_CELL_17_TO_32);
    for (uint16_t i = 16; i < std::min<uint16_t>(this->cell_balancing_sensors_max_, 32); i++) {
      if (this->cell_balancing_sensors_[i] != nullptr)
        this->cell_balancing_sensors_[i]->publish_state(reg_val & (1 << (i-16)));
    }

    reg_val = get_register(DALY_MODBUS_ADDR_BALANCE_STATUS_PER_CELL_33_TO_48);
    for (uint16_t i = 32; i < std::min<uint16_t>(this->cell_balancing_sensors_max_, 48); i++) {
      if (this->cell_balancing_sensors_[i] != nullptr)
        this->cell_balancing_sensors_[i]->publish_state(reg_val & (1 << (i-32)));
    }
  }  

  if (this->charging_mos_enabled_binary_sensor_ && has_register(DALY_MODBUS_ADDR_CHG_MOS_ACTIVE)) {
    this->charging_mos_enabled_binary_sensor_->publish_state(get_register(DALY_MODBUS_ADDR_CHG_MOS_ACTIVE) > 0);
  }
  if (this->discharging_mos_enabled_binary_sensor_ && has_register(DALY_MODBUS_ADDR_DSCHG_MOS_ACTIVE)) {
    this->discharging_mos_enabled_binary_sensor_->publish_state(get_register(DALY_MODBUS_ADDR_DSCHG_MOS_ACTIVE) > 0);
  }
  if (this->precharging_mos_enabled_binary_sensor_ && has_register(DALY_MODBUS_ADDR_PRECHG_MOS_ACTIVE)) {
    this->precharging_mos_enabled_binary_sensor_->publish_state(get_register(DALY_MODBUS_ADDR_PRECHG_MOS_ACTIVE) > 0);
  }

  if (has_status) {
    bool has_warnings = false;
    bool has_errors = false;

    // Code 0-1
    has_warnings |= (status.lvl_cell_ovp) > 0;
    has_errors |= (status.lvl_cell_ovp) > 1;
    has_warnings |= (status.lvl_cell_uvp) > 0;
    has_errors |= (status.lvl_cell_uvp) > 1;
    has_errors |= status.err_smart_charger_connection;

    has_warnings |= (status.lvl_cell_volt_diff) > 0;
    has_errors |= (status.lvl_cell_volt_diff) > 1;
    has_warnings |= (status.lvl_chg_overtemp) > 0;
    has_errors |= (status.lvl_chg_overtemp) > 1;
    has_errors |= status.err_smart_discharger_connection;

    // Code 2-3
    has_warnings |= (status.lvl_chg_undertemp) > 0;
    has_errors |= (status.lvl_chg_undertemp) > 1;

    has_warnings |= (status.lvl_dschg_overtemp) > 0;
    has_errors |= (status.lvl_dschg_overtemp) > 1;

    has_errors |= status.err_chg_mos_temp_high;
    publish_binary_sensor_state(this->error_charge_mos_overtemperature_binary_sensor_, status.err_chg_mos_temp_high);
    
    has_errors |= status.err_chg_mos_temp_detect;
    publish_binary_sensor_state(this->error_charge_mos_temperature_detect_binary_sensor_, status.err_chg_mos_temp_detect);

    has_errors |= status.err_dschg_mos_temp_high;
    publish_binary_sensor_state(this->error_discharge_mos_overtemperature_binary_sensor_, status.err_dschg_mos_temp_high);
    
    has_errors |= status.err_dschg_mos_temp_detect;
    publish_binary_sensor_state(this->error_discharge_mos_temperature_detect_binary_sensor_, status.err_dschg_mos_temp_detect);
    
    // Code 4-5
    has_warnings |= (status.lvl_total_ovp) > 0;
    has_errors |= (status.lvl_total_ovp) > 1;

    has_warnings |= (status.lvl_total_uvp) > 0;
    has_errors |= (status.lvl_total_uvp) > 1;

    has_errors |= status.err_short_circuit;
    publish_binary_sensor_state(this->error_short_circuit_binary_sensor_, status.err_short_circuit);
    
    has_warnings |= (status.lvl_chg_ocp) > 0;
    has_errors |= (status.lvl_chg_ocp) > 1;

    has_warnings |= (status.lvl_dschg_ocp) > 0;
    has_errors |= (status.lvl_dschg_ocp) > 1;
    
    has_errors |= status.err_chg_undervoltage;
    has_errors |= status.err_dschg_overvoltage;

    // Code 6-7
    // SoC and SoH are never errors
    has_warnings |= (status.lvl_soc_low) > 0;
    has_warnings |= (status.lvl_soh_low) > 0;

    has_errors |= status.err_parallel_comm;

    has_warnings |= (status.lvl_mos_overtemp) > 0;
    has_errors |= (status.lvl_mos_overtemp) > 1;

    has_warnings |= (status.lvl_thermal_runaway) > 0;
    has_errors |= (status.lvl_thermal_runaway) > 1;

    // Code 10-11
    has_errors |= status.err_afe_chip;
    has_errors |= status.err_afe_comm;
    has_errors |= status.err_afe_sampling;
    has_errors |= status.err_volt_detect;
    has_errors |= status.err_volt_detect_disconnected;
    has_errors |= status.err_volt_total_detect;
    has_errors |= status.err_curr_detect;
    has_errors |= status.err_temp_detect;

    // Code 12-13
    has_errors |= status.err_temp_disconnected;
    has_errors |= status.err_eeprom;
    has_errors |= status.err_flash;
    has_errors |= status.err_rtc;
    has_errors |= status.err_chg_mos;
    has_errors |= status.err_dschg_mos;
    has_errors |= status.err_prechg_mos;
    has_errors |= status.err_prechg;

    has_errors |= status.err_heating;

    publish_binary_sensor_state(this->has_warnings_binary_sensor_, has_warnings || has_errors);
    publish_binary_sensor_state(this->has_errors_binary_sensor_, has_errors);
  }
#endif

#ifdef USE_TEXT_ALERTS

  if (has_status && this->alerts_text_sensor_ != nullptr) {
    std::ostringstream alerts_buffer;

    // Code 0-1
    if (status.lvl_cell_ovp > 0) {
      alerts_buffer << "cell volt high lvl " << int(status.lvl_cell_ovp) << "\n";
    }
    if (status.lvl_cell_uvp > 0) {
      alerts_buffer << "cell volt low lvl " << int(status.lvl_cell_uvp) << "\n";
    }
    if (status.lvl_cell_volt_diff > 0) {
      alerts_buffer << "cell volt diff lvl " << int(status.lvl_cell_volt_diff) << "\n";
    }
    if (status.lvl_chg_overtemp > 0) {
      alerts_buffer << "chg temp high lvl " << int(status.lvl_chg_overtemp) << "\n";
    }

    if (status.smart_charger_connected) {
      alerts_buffer << "smart charger connected\n";
    }
    if (status.err_smart_charger_connection) {
      alerts_buffer << "smart charger disconnected\n";
    }

    if (status.smart_discharger_connected) {
      alerts_buffer << "smart discharger connected\n";
    }
    if (status.err_smart_discharger_connection) {
      alerts_buffer << "smart discharger disconnected\n";
    }

    // Code 2-3
    if (status.lvl_chg_undertemp > 0) {
      alerts_buffer << "chg temp low lvl " << int(status.lvl_chg_undertemp) << "\n";
    }
    if (status.lvl_dschg_overtemp > 0) {
      alerts_buffer << "dschg temp high lvl " << int(status.lvl_dschg_overtemp) << "\n";
    }
    if (status.lvl_dschg_undertemp > 0) {
      alerts_buffer << "dschg temp low lvl " << int(status.lvl_dschg_undertemp) << "\n";
    }
    if (status.lvl_temp_diff > 0) {
      alerts_buffer << "temp diff lvl " << int(status.lvl_temp_diff) << "\n";
    }

    if (status.err_chg_mos_temp_high) {
      alerts_buffer << "chg mos temp high\n";
    }
    if (status.err_chg_mos_temp_detect) {
      alerts_buffer << "chg mos temp detect fault\n";
    }

    if (status.err_dschg_mos_temp_high) {
      alerts_buffer << "dschg mos temp high\n";
    }
    if (status.err_dschg_mos_temp_detect) {
      alerts_buffer << "dschg mos temp detect fault\n";
    }

    // Code 4-5
    if (status.lvl_total_ovp > 0) {
      alerts_buffer << "total volt high lvl " << int(status.lvl_total_ovp) << "\n";
    }
    if (status.lvl_total_uvp > 0) {
      alerts_buffer << "total volt low lvl " << int(status.lvl_total_uvp) << "\n";
    }
    if (status.lvl_chg_ocp > 0) {
      alerts_buffer << "chg curr high lvl " << int(status.lvl_chg_ocp) << "\n";
    }
    if (status.lvl_dschg_ocp > 0) {
      alerts_buffer << "dschg curr high lvl " << int(status.lvl_dschg_ocp) << "\n";
    }

    if (status.err_short_circuit) {
      alerts_buffer << "short circuit protect\n";
    }
    if (status.upgrade_sign) {
      alerts_buffer << "upgrade sign\n";
    }

    if (status.err_chg_undervoltage) {
      alerts_buffer << "charge undervoltage\n";
    }
    if (status.err_dschg_overvoltage) {
      alerts_buffer << "discharge overvoltage\n";
    }

    // Code 6-7
    if (status.lvl_soc_low > 0) {
      alerts_buffer << "soc low lvl " << int(status.lvl_soc_low) << "\n";
    }
    if (status.lvl_soh_low > 0) {
      alerts_buffer << "soh low lvl " << int(status.lvl_soh_low) << "\n";
    }
    if (status.lvl_mos_overtemp > 0) {
      alerts_buffer << "mos temp high lvl " << int(status.lvl_mos_overtemp) << "\n";
    }
    if (status.lvl_thermal_runaway > 0) {
      alerts_buffer << "thermal runaway lvl " << int(status.lvl_thermal_runaway) << "\n";
    }

    if (status.parallel_comm) {
      alerts_buffer << "parallel comm ok\n";
    }
    if (status.err_parallel_comm) {
      alerts_buffer << "parallel comm fault\n";
    }

    // Code 10-11
    if (status.err_afe_chip) {
      alerts_buffer << "afe ic fault\n";
    }
    if (status.err_afe_comm) {
      alerts_buffer << "afe ic comm fault\n";
    }
    if (status.err_afe_sampling) {
      alerts_buffer << "afe ic ad fault\n";
    }
    if (status.err_volt_detect) {
      alerts_buffer << "cell volt detect fault\n";
    }
    if (status.err_volt_detect_disconnected) {
      alerts_buffer << "cell volt detect disconnected\n";
    }
    if (status.err_volt_total_detect) {
      alerts_buffer << "total volt detect fault\n";
    }
    if (status.err_curr_detect) {
      alerts_buffer << "curr detect fault\n";
    }
    if (status.err_temp_detect) {
      alerts_buffer << "temp detect fault\n";
    }

    // Code 12-13
    if (status.err_temp_disconnected) {
      alerts_buffer << "temp detect disconnected\n";
    }
    if (status.err_eeprom) {
      alerts_buffer << "EEPROM fault\n";
    }
    if (status.err_flash) {
      alerts_buffer << "flash fault\n";
    }
    if (status.err_rtc) {
      alerts_buffer << "RTC fault\n";
    }
    if (status.err_chg_mos) {
      alerts_buffer << "chg mos fault\n";
    }
    if (status.err_dschg_mos) {
      alerts_buffer << "dschg mos fault\n";
    }
    if (status.err_prechg_mos) {
      alerts_buffer << "prechg mos fault\n";
    }
    if (status.err_prechg) {
      alerts_buffer << "prechg failed\n";
    }

    if (status.chg_mos_off_bus) {
      alerts_buffer << "chg mos off (via comm)\n";
    }
    if (status.dschg_mos_off_bus) {
      alerts_buffer << "dschg mos off (via comm)\n";
    }
    if (status.chg_mos_off_switch) {
      alerts_buffer << "chg mos off (via switch)\n";
    }
    if (status.dschg_mos_off_switch) {
      alerts_buffer << "dschg mos off (via switch)\n";
    }
    if (status.fan_active) {
      alerts_buffer << "fan active\n";
    }
    if (status.heating_active) {
      alerts_buffer << "heater active\n";
    }
    if (status.current_limit_active) {
      alerts_buffer << "current limit active\n";
    }
    if (status.err_heating) {
      alerts_buffer << "heater fault\n";
    }

    std::string alerts_str = alerts_buffer.str();
    if (!alerts_str.empty())
      alerts_str.pop_back();

    this->alerts_text_sensor_->publish_state(alerts_str);
  }
#endif

  for (auto &input : this->registered_inputs_) {
    uint16_t address = input->get_reg_addr();
    if (has_register(address))
      input->handle_update(get_register(address));
  }
}

void DalyHkmsBmsComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "DALY HKMS BMS:");
  ESP_LOGCONFIG(TAG, "  Address: 0x%02X", this->daly_address_);
}

}  // namespace daly_hkms_bms
}  // namespace esphome
