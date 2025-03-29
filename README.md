# esphome-daly-hkms-bms
ESPHome component to monitor DALY H/K/M/S-Series Battery Management Systems via UART / RS485.  
Works with other of the "new" DALY BMS as well.

Tested with:
- 100A 15S K-Series BMS
- 100A 8S K-Series BMS
- 500A 24S "100 Balance" Active Balance BMS

Also see https://github.com/syssi/esphome-daly-bms for a similar component that uses BLE.

## Configuration

### Main component
```yaml
daly_hkms_bms:
  - id: bms_1
    modbus_id: bms_modbus
    address: 1
    update_interval: 10s
```

There can be multiple BMS attached to one modbus. To make this work, each BMS needs its own address which can be set in the DALY BMS Tool (Administrator -> Password `12345678` -> Manufacturing -> Board number).

#### Options:
- **id**: ID of this component
- **modbus_id**: ID of the [modbus component](https://esphome.io/components/modbus.html) the BMS is attached to
- **address**: The address / board number of the BMS
- **update_interval**: Delay between data requests (default `30s`)
- **update_interval_fast**:
    Delay between interjected "fast" data requests (includes only voltage and current, default: off).
    Be aware that fast requests always have priority, so if normal updates dont (always) work, this might be set too low.

### Sensor component

```yaml
sensor:
  - platform: daly_hkms_bms
    daly_hkms_bms_id: bms_1
    voltage:
      name: "Battery Voltage"
    current:
      name: "Battery Current"
    battery_level:
      name: "Battery Level"
    remaining_capacity:
      name: "Remaining Capacity"
    cycles:
      name: "Battery Cycles"
    temperature_mos:
      name: "Temperature MOS"
    max_temperature:
      name: "Temperature Max"
    min_temperature:
      name: "Temperature Min"
    max_cell_voltage:
      name: "Cell Voltage Max"
    min_cell_voltage:
      name: "Cell Voltage Min"
```

#### Options:

- **daly_hkms_bms_id**: ID of the daly_hkms_bms component (required for multiple BMS).
- **voltage**: Voltage of the battery pack.
- **current**: Current flowing through the BMS (positive when charging, negative when discharging).
- **battery_level**: Battery level in % (SoC).
- **remaining_capacity**: The capacity in Ah left in the battery.
- **cycles**: The number of charge cycles of the battery.
- **balance_current** (not supported on all BMS versions)
- **power**: Power (positive when charging, negative when discharging)
- **charge_power**: Charge power (always positive)
- **discharge_power**: Discharge power (always positive)
- **energy**: (not supported on all BMS versions)
- **temperature_mos**: The BMS MOSFET temperature.
- **temperature_board**: The BMS PCB temperature (might not be available on all models).
- **temps_number**: The number of temperature sensors.
- **max_temperature**: The highest temperature measured from the temperature sensors.
- **max_temperature_probe_number**: The sensor number which has measured the highest temperature.
- **min_temperature**: The lowest temperature measured from the temperature sensors.
- **min_temperature_probe_number**: The sensor number which has measured the lowest temperature.
- **temperature_1**: The first temperature sensor. There can be up to 8 temperature sensors.
- **cells_number**: The number of cells in series in the battery pack.
- **max_cell_voltage**: The cell of the battery with the highest voltage.
- **max_cell_voltage_number**: The cell number of the battery with the highest voltage.
- **min_cell_voltage**: The cell of the battery with the lowest voltage.
- **min_cell_voltage_number**: The cell number of the battery with the lowest voltage.
- **cell_1_voltage**: The voltage of cell number 1. Cell number can be from 1 to 48.
- **alarm_level_cell_overvoltage_sensor**
- **alarm_level_cell_undervoltage_sensor**
- **alarm_level_cell_voltage_diff_sensor**
- **alarm_level_charge_overtemperature_sensor**
- **alarm_level_charge_undertemperature_sensor**
- **alarm_level_discharge_overtemperature_sensor**
- **alarm_level_discharge_undertemperature_sensor**
- **alarm_level_temperature_diff_sensor**
- **alarm_level_overvoltage_sensor**
- **alarm_level_undervoltage_sensor**
- **alarm_level_charge_overcurrent_sensor**
- **alarm_level_discharge_overcurrent_sensor**
- **alarm_level_soc_low_sensor**
- **alarm_level_soh_low_sensor**
- **alarm_level_mos_overtemperature_sensor**

### Text Sensor component

```yaml
text_sensor:
  - platform: daly_hkms_bms
    daly_hkms_bms_id: bms_1
    status:
      name: "BMS Status"
```

#### Options:
- **daly_hkms_bms_id**: ID of the daly_hkms_bms component (required for multiple BMS).
- **status**: The BMS status (Charging, Discharging, Stationary).

### Binary Sensor component

```yaml
binary_sensor:
  - platform: daly_hkms_bms
    daly_hkms_bms_id: bms_1
    balancing_active:
      name: "BMS balancing"
    charging_mos_enabled:
      name: "BMS charging FET enabled"
    discharging_mos_enabled:
      name: "BMS discharging FET enabled"
    has_errors:
      name: "BMS has errors/warnings"
```

#### Options:
- **daly_hkms_bms_id**: ID of the daly_hkms_bms component (required for multiple BMS).
- **balancing_active**: Whether the BMS is currently balancing or not.
- **cell_1_balancing**: Whether the cell is currently being balanced or not. Cell number can be from 1 to 48.
- **charging_mos_enabled**: BMS charging MOS status.
- **discharging_mos_enabled**: BMS discharging MOS status.
- **precharging_mos_enabled**: BMS precharging MOS status.
- **has_errors**: BMS warning/error status.
- **error_charge_mos_overtemperature_binary_sensor**
- **error_charge_mos_temperature_detect_binary_sensor**
- **error_discharge_mos_overtemperature_binary_sensor**
- **error_discharge_mos_temperature_detect_binary_sensor**
- **error_short_circuit_binary_sensor**

### Switch component

```yaml
switch:
  - platform: daly_hkms_bms
    daly_hkms_bms_id: bms_1
    charge_mos:
      name: "BMS charge FET"
    discharge_mos:
      name: "BMS discharge FET"
```

#### Options:
- **daly_hkms_bms_id**: ID of the daly_hkms_bms component (required for multiple BMS).
- **charge_mos**: BMS charging MOS switch.
- **discharge_mos**: BMS discharging MOS switch.

### Full Example:
```yaml
external_components:
  - source: github://patagonaa/esphome-daly-hkms-bms@main
    components: [daly_hkms_bms]

uart:
  - id: bms_uart
    baud_rate: 9600
    rx_pin: GPIO21
    tx_pin: GPIO22

modbus:
  id: bms_modbus
  uart_id: bms_uart
  flow_control_pin: GPIO17 # not required if your RS485 transceiver has auto direction control (only has RX/TX)

daly_hkms_bms:
  - modbus_id: bms_modbus
    id: bms_1
    address: 1
    update_interval: 10s

sensor:
  - platform: daly_hkms_bms
    daly_hkms_bms_id: bms_1
    voltage:
      name: "BMS total volt"
    current:
      name: "BMS current"
    battery_level:
      name: "BMS state of charge"
    remaining_capacity:
      name: "BMS capacity remaining"
    cycles:
      name: "BMS charging cycles"
    min_cell_voltage:
      name: "BMS cell min volt"
    max_cell_voltage:
      name: "BMS cell max volt"
    delta_cell_voltage:
      name: "BMS cell delta volt"
    temperature_1:
      name: "BMS temperature 1"
    temperature_2:
      name: "BMS temperature 2"
    cell_1_voltage:
      name: "BMS cell volt 01"
    cell_2_voltage:
      name: "BMS cell volt 02"
    # [...]

text_sensor:
  - platform: daly_hkms_bms
    daly_hkms_bms_id: bms_1
    status:
      name: "BMS Status"

binary_sensor:
  - platform: daly_hkms_bms
    daly_hkms_bms_id: bms_1
    balancing_active:
      name: "BMS balancing"
    charging_mos_enabled:
      name: "BMS charging FET enabled"
    discharging_mos_enabled:
      name: "BMS discharging FET enabled"

switch:
  - platform: daly_hkms_bms
    daly_hkms_bms_id: bms_1
    charge_mos:
      name: "BMS charge FET"
    discharge_mos:
      name: "BMS discharge FET"
```

## BMS connection
Be aware that by default, the BMS goes to sleep after 1 hour of inactivity and can not be woken up via RS485, only by RX/TX UART communication (including the Bluetooth dongle), charging/discharging the battery or toggling the switch input.

The BMS connectors are compatible with JST-GH (1.25mm pin pitch).

The BMS can be connected via UART or RS485. RS485 is less susceptible to interference and multiple BMS can be hooked up to one RS485 bus.

### UART

> [!WARNING]  
> The UART ground is always connected to the battery ground (not the BMS output ground)!  
> If the ESP is connected to the output side of the battery (through a DC-DC converter, GPIO, etc.) the ESP and/or BMS will be damaged once the BMS switches off!

Ideally, the ESP should be powered by the 3.3V on the BMS connector and not connected to anything else (unless opto-isolated).

#### ESP Connections
- tx_pin: BMS RX
- rx_pin: BMS TX
- GND: BMS GND

### RS485
In an RS485 bus, no pins are swapped between sender and receiver(s) (A -> A, B -> B, GND -> GND).

Also, the RS485 outputs of the BMS are isolated from the battery (at least on the K-series), so there are no ground issues like the UART pins have.

Still, always connect the ground when using RS485, to avoid stray currents running through the RS485 transceiver.

#### ESP Connections
- Transceiver without auto direction control (R, D, NRE, DE)
    - tx_pin: D
    - rx_pin: R
    - flow_control_pin: NRE, DE (tied together)
- Transceiver with auto direction control (RX, TX)
    - tx_pin: TX
    - rx_pin: RX
