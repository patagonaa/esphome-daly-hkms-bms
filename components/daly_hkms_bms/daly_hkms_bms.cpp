#include "daly_hkms_bms.h"
#include "daly_hkms_bms_registers.h"
#include "esphome/core/log.h"

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
      ESP_LOGD(TAG, "Sending modbus read request to %d: start register %" PRIu16 ", register count %" PRIu16, this->daly_address_,
        to_send.register_address, to_send.data);
      this->parent_->send(modbus_device_request_address, MODBUS_CMD_READ_HOLDING_REGISTERS, to_send.register_address, to_send.data,
        0, nullptr);
      break;
    }
    case MODBUS_CMD_WRITE_SINGLE_REGISTER:
    {
      ESP_LOGD(TAG, "Sending modbus write request to %d: register %" PRIu16 ", value %" PRIu16, this->daly_address_,
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

  ESP_LOGD(TAG, "#%d Got modbus response: %d bytes", this->daly_address_, data.size());

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

  auto publish_sensor_state = [&](sensor::Sensor *sensor, uint16_t i, int16_t offset, float factor,
                                  int32_t unavailable_value = -1) -> void {
    if (sensor == nullptr || !has_register(i))
      return;
    uint16_t register_value = get_register(i);
    float value = register_value == unavailable_value ? NAN : (register_value + offset) * factor;
    sensor->publish_state(value);
  };


#ifdef USE_SENSOR
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
    uint16_t chg_dschg_status = get_register(DALY_MODBUS_ADDR_CHG_DSCHG_STATUS);
    switch (chg_dschg_status) {
      case 0: // stationary
        publish_sensor_state(this->power_sensor_, DALY_MODBUS_ADDR_POWER, 0, 0);
        publish_sensor_state(this->charge_power_sensor_, DALY_MODBUS_ADDR_POWER, 0, 0);
        publish_sensor_state(this->discharge_power_sensor_, DALY_MODBUS_ADDR_POWER, 0, 0);
        break;

      case 1: // charging
        publish_sensor_state(this->power_sensor_, DALY_MODBUS_ADDR_POWER, 0, 1);
        publish_sensor_state(this->charge_power_sensor_, DALY_MODBUS_ADDR_POWER, 0, 1);
        publish_sensor_state(this->discharge_power_sensor_, DALY_MODBUS_ADDR_POWER, 0, 0);
        break;

      case 2: // discharging
        publish_sensor_state(this->power_sensor_, DALY_MODBUS_ADDR_POWER, 0, -1);
        publish_sensor_state(this->charge_power_sensor_, DALY_MODBUS_ADDR_POWER, 0, 0);
        publish_sensor_state(this->discharge_power_sensor_, DALY_MODBUS_ADDR_POWER, 0, 1);
        break;
      
      default:
        break;
    }
  }

  publish_sensor_state(this->energy_sensor_, DALY_MODBUS_ADDR_ENERGY, 0, 1);

  publish_sensor_state(this->temperature_mos_sensor_, DALY_MODBUS_ADDR_MOS_TEMP, -40, 1, 255);
  publish_sensor_state(this->temperature_board_sensor_, DALY_MODBUS_ADDR_BOARD_TEMP, -40, 1, 255);


  if (has_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_00_01) && has_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_12_13)) {
    uint16_t reg_val = get_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_00_01);

    if (this->cell_overvoltage_alarm_level_sensor_ != nullptr)
      this->cell_overvoltage_alarm_level_sensor_->publish_state(reg_val & 0x07);

    if (this->cell_undervoltage_alarm_level_sensor_ != nullptr)
      this->cell_undervoltage_alarm_level_sensor_->publish_state((reg_val >> 3) & 0x07);
        
    if (this->cell_voltage_diff_alarm_level_sensor_ != nullptr)
      this->cell_voltage_diff_alarm_level_sensor_->publish_state((reg_val >> 8) & 0x07);

    // charge overtemp

    reg_val = get_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_02_03);

    // charge undertemp

    // discharge overtemp

    // discharge undertemp

    // temp difference
    
    reg_val = get_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_04_05);

    if (this->overvoltage_alarm_level_sensor_ != nullptr)
      this->overvoltage_alarm_level_sensor_->publish_state(reg_val & 0x07);

    if (this->undervoltage_alarm_level_sensor_ != nullptr)
      this->undervoltage_alarm_level_sensor_->publish_state((reg_val >> 3) & 0x07);
        
    if (this->charge_overcurrent_alarm_level_sensor_ != nullptr)
      this->charge_overcurrent_alarm_level_sensor_->publish_state((reg_val >> 8) & 0x07);
        
    if (this->discharge_overcurrent_alarm_level_sensor_ != nullptr)
      this->discharge_overcurrent_alarm_level_sensor_->publish_state((reg_val >> 11) & 0x07);
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

  if (this->has_errors_binary_sensor_ && has_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_00_01) && has_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_12_13)) {
    bool has_errors = false;

    uint16_t reg_val = get_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_00_01);
    has_errors |= reg_val & 0x3F3F;

    reg_val = get_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_02_03);
    has_errors |= reg_val & 0x3F3F;
    
    reg_val = get_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_04_05);
    has_errors |= reg_val & 0xFF7F;

    reg_val = get_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_06_07);
    has_errors |= reg_val & 0x3F3F;

    reg_val = get_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_08_09);

    reg_val = get_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_10_11);
    has_errors |= reg_val & 0xFF00;

    reg_val = get_register(DALY_MODBUS_ADDR_BMS_TYPE_2_ERR_12_13);
    has_errors |= reg_val & 0x80FF;

    this->has_errors_binary_sensor_->publish_state(has_errors);
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
