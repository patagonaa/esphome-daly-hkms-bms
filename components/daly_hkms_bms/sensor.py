import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_VOLTAGE,
    CONF_CURRENT,
    CONF_BATTERY_LEVEL,
    CONF_MAX_TEMPERATURE,
    CONF_MIN_TEMPERATURE,
    CONF_POWER,
    CONF_ENERGY,
    DEVICE_CLASS_VOLTAGE,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_EMPTY,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_NONE,
    UNIT_EMPTY,
    UNIT_VOLT,
    UNIT_AMPERE,
    UNIT_PERCENT,
    UNIT_CELSIUS,
    UNIT_WATT,
    UNIT_WATT_HOURS,
    ICON_FLASH,
    ICON_PERCENT,
    ICON_COUNTER,
    ICON_THERMOMETER,
    ICON_GAUGE,
)
from . import MAX_CELL_NUMBER, DalyHkmsBmsComponent, CONF_DALY_HKMS_BMS_ID

CONF_MAX_CELL_VOLTAGE = "max_cell_voltage"
CONF_MAX_CELL_VOLTAGE_NUMBER = "max_cell_voltage_number"
CONF_MIN_CELL_VOLTAGE = "min_cell_voltage"
CONF_MIN_CELL_VOLTAGE_NUMBER = "min_cell_voltage_number"
CONF_DELTA_CELL_VOLTAGE = "delta_cell_voltage"
CONF_MAX_TEMPERATURE_PROBE_NUMBER = "max_temperature_probe_number"
CONF_MIN_TEMPERATURE_PROBE_NUMBER = "min_temperature_probe_number"
CONF_CELLS_NUMBER = "cells_number"
CONF_TEMPS_NUMBER = "temps_number"

CONF_REMAINING_CAPACITY = "remaining_capacity"
CONF_CYCLES = "cycles"
CONF_BALANCE_CURRENT = "balance_current"

CONF_CHARGE_POWER = "charge_power"
CONF_DISCHARGE_POWER = "discharge_power"

CONF_TEMPERATURE_MOS = "temperature_mos"
CONF_TEMPERATURE_BOARD = "temperature_board"

CONF_ALARM_LEVEL_CELL_OVERVOLTAGE = "alarm_level_cell_overvoltage"
CONF_ALARM_LEVEL_CELL_UNDERVOLTAGE = "alarm_level_cell_undervoltage"
CONF_ALARM_LEVEL_CELL_VOLTAGE_DIFF = "alarm_level_cell_voltage_diff"
CONF_ALARM_LEVEL_CHARGE_OVERTEMPERATURE = "alarm_level_charge_overtemperature"
CONF_ALARM_LEVEL_CHARGE_UNDERTEMPERATURE = "alarm_level_charge_undertemperature"
CONF_ALARM_LEVEL_DISCHARGE_OVERTEMPERATURE = "alarm_level_discharge_overtemperature"
CONF_ALARM_LEVEL_DISCHARGE_UNDERTEMPERATURE = "alarm_level_discharge_undertemperature"
CONF_ALARM_LEVEL_TEMPERATURE_DIFF = "alarm_level_temperature_diff"
CONF_ALARM_LEVEL_OVERVOLTAGE = "alarm_level_overvoltage"
CONF_ALARM_LEVEL_UNDERVOLTAGE = "alarm_level_undervoltage"
CONF_ALARM_LEVEL_CHARGE_OVERCURRENT = "alarm_level_charge_overcurrent"
CONF_ALARM_LEVEL_DISCHARGE_OVERCURRENT = "alarm_level_discharge_overcurrent"
CONF_ALARM_LEVEL_SOC_LOW = "alarm_level_soc_low"
CONF_ALARM_LEVEL_SOH_LOW = "alarm_level_soh_low"
CONF_ALARM_LEVEL_MOS_OVERTEMPERATURE = "alarm_level_mos_overtemperature"

ICON_CURRENT_DC = "mdi:current-dc"
ICON_BATTERY_OUTLINE = "mdi:battery-outline"
ICON_THERMOMETER_CHEVRON_UP = "mdi:thermometer-chevron-up"
ICON_THERMOMETER_CHEVRON_DOWN = "mdi:thermometer-chevron-down"
ICON_CAR_BATTERY = "mdi:car-battery"
ICON_SCALE_BALANCE = "mdi:scale-balance"
ICON_BATTERY_ALERT = "mdi:battery-alert"

UNIT_AMPERE_HOUR = "Ah"

MAX_TEMP_NUMBER = 8

TYPES = [
    CONF_VOLTAGE,
    CONF_CURRENT,
    CONF_BATTERY_LEVEL,
    CONF_MAX_CELL_VOLTAGE,
    CONF_MAX_CELL_VOLTAGE_NUMBER,
    CONF_MIN_CELL_VOLTAGE,
    CONF_MIN_CELL_VOLTAGE_NUMBER,
    CONF_DELTA_CELL_VOLTAGE,
    CONF_MAX_TEMPERATURE,
    CONF_MAX_TEMPERATURE_PROBE_NUMBER,
    CONF_MIN_TEMPERATURE,
    CONF_MIN_TEMPERATURE_PROBE_NUMBER,
    CONF_CELLS_NUMBER,
    CONF_TEMPS_NUMBER,
    CONF_REMAINING_CAPACITY,
    CONF_CYCLES,
    CONF_BALANCE_CURRENT,
    CONF_POWER,
    CONF_CHARGE_POWER,
    CONF_DISCHARGE_POWER,
    CONF_ENERGY,
    CONF_TEMPERATURE_MOS,
    CONF_TEMPERATURE_BOARD,
    CONF_ALARM_LEVEL_CELL_OVERVOLTAGE,
    CONF_ALARM_LEVEL_CELL_UNDERVOLTAGE,
    CONF_ALARM_LEVEL_CELL_VOLTAGE_DIFF,
    CONF_ALARM_LEVEL_CHARGE_OVERTEMPERATURE,
    CONF_ALARM_LEVEL_CHARGE_UNDERTEMPERATURE,
    CONF_ALARM_LEVEL_DISCHARGE_OVERTEMPERATURE,
    CONF_ALARM_LEVEL_DISCHARGE_UNDERTEMPERATURE,
    CONF_ALARM_LEVEL_TEMPERATURE_DIFF,
    CONF_ALARM_LEVEL_OVERVOLTAGE,
    CONF_ALARM_LEVEL_UNDERVOLTAGE,
    CONF_ALARM_LEVEL_CHARGE_OVERCURRENT,
    CONF_ALARM_LEVEL_DISCHARGE_OVERCURRENT,
    CONF_ALARM_LEVEL_SOC_LOW,
    CONF_ALARM_LEVEL_SOH_LOW,
    CONF_ALARM_LEVEL_MOS_OVERTEMPERATURE,
    # Cell voltages and temperatures are handled by loops below
]

TEMPERATURE_SENSOR_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    icon=ICON_THERMOMETER,
    accuracy_decimals=0,
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
)

CELL_VOLTAGE_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    device_class=DEVICE_CLASS_VOLTAGE,
    state_class=STATE_CLASS_MEASUREMENT,
    icon=ICON_FLASH,
    accuracy_decimals=3,
)


def get_cell_voltage_key(cell):
    return f"cell_{cell}_voltage"


def get_cell_voltages_schema():
    schema_obj = {}
    for i in range(1, MAX_CELL_NUMBER + 1):
        schema_obj[cv.Optional(get_cell_voltage_key(i))] = CELL_VOLTAGE_SCHEMA
    return cv.Schema(schema_obj)


def get_temperature_sensor_key(temp):
    return f"temperature_{temp}"


def get_temperature_sensors_schema():
    schema_obj = {}
    for i in range(1, MAX_TEMP_NUMBER + 1):
        schema_obj[cv.Optional(get_temperature_sensor_key(i))] = TEMPERATURE_SENSOR_SCHEMA
    return cv.Schema(schema_obj)

ALARM_LEVEL_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_EMPTY,
    icon=ICON_BATTERY_ALERT,
    accuracy_decimals=0,
    device_class=DEVICE_CLASS_EMPTY,
    state_class=STATE_CLASS_NONE,
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(CONF_DALY_HKMS_BMS_ID): cv.use_id(DalyHkmsBmsComponent),
            cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                icon=ICON_CURRENT_DC,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BATTERY_LEVEL): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                icon=ICON_PERCENT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_BATTERY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MAX_CELL_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                icon=ICON_FLASH,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MAX_CELL_VOLTAGE_NUMBER): sensor.sensor_schema(
                unit_of_measurement=UNIT_EMPTY,
                icon=ICON_COUNTER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MIN_CELL_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                icon=ICON_FLASH,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MIN_CELL_VOLTAGE_NUMBER): sensor.sensor_schema(
                unit_of_measurement=UNIT_EMPTY,
                icon=ICON_COUNTER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_DELTA_CELL_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                icon=ICON_FLASH,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MAX_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                icon=ICON_THERMOMETER_CHEVRON_UP,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MAX_TEMPERATURE_PROBE_NUMBER): sensor.sensor_schema(
                unit_of_measurement=UNIT_EMPTY,
                icon=ICON_COUNTER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MIN_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                icon=ICON_THERMOMETER_CHEVRON_DOWN,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MIN_TEMPERATURE_PROBE_NUMBER): sensor.sensor_schema(
                unit_of_measurement=UNIT_EMPTY,
                icon=ICON_COUNTER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_REMAINING_CAPACITY): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE_HOUR,
                icon=ICON_GAUGE,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CYCLES): sensor.sensor_schema(
                unit_of_measurement=UNIT_EMPTY,
                icon=ICON_COUNTER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_BALANCE_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                icon=ICON_SCALE_BALANCE,
                accuracy_decimals=3,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CELLS_NUMBER): sensor.sensor_schema(
                icon=ICON_COUNTER,
                accuracy_decimals=0,
            ),
            cv.Optional(CONF_TEMPS_NUMBER): sensor.sensor_schema(
                icon=ICON_COUNTER,
                accuracy_decimals=0,
            ),
            cv.Optional(CONF_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CHARGE_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_DISCHARGE_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ENERGY): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT_HOURS,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_ENERGY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE_MOS): TEMPERATURE_SENSOR_SCHEMA,
            cv.Optional(CONF_TEMPERATURE_BOARD): TEMPERATURE_SENSOR_SCHEMA,

            cv.Optional(CONF_ALARM_LEVEL_CELL_OVERVOLTAGE): ALARM_LEVEL_SCHEMA,
            cv.Optional(CONF_ALARM_LEVEL_CELL_UNDERVOLTAGE): ALARM_LEVEL_SCHEMA,
            cv.Optional(CONF_ALARM_LEVEL_CELL_VOLTAGE_DIFF): ALARM_LEVEL_SCHEMA,
            cv.Optional(CONF_ALARM_LEVEL_CHARGE_OVERTEMPERATURE): ALARM_LEVEL_SCHEMA,
            cv.Optional(CONF_ALARM_LEVEL_CHARGE_UNDERTEMPERATURE): ALARM_LEVEL_SCHEMA,
            cv.Optional(CONF_ALARM_LEVEL_DISCHARGE_OVERTEMPERATURE): ALARM_LEVEL_SCHEMA,
            cv.Optional(CONF_ALARM_LEVEL_DISCHARGE_UNDERTEMPERATURE): ALARM_LEVEL_SCHEMA,
            cv.Optional(CONF_ALARM_LEVEL_TEMPERATURE_DIFF): ALARM_LEVEL_SCHEMA,
            cv.Optional(CONF_ALARM_LEVEL_OVERVOLTAGE): ALARM_LEVEL_SCHEMA,
            cv.Optional(CONF_ALARM_LEVEL_UNDERVOLTAGE): ALARM_LEVEL_SCHEMA,
            cv.Optional(CONF_ALARM_LEVEL_CHARGE_OVERCURRENT): ALARM_LEVEL_SCHEMA,
            cv.Optional(CONF_ALARM_LEVEL_DISCHARGE_OVERCURRENT): ALARM_LEVEL_SCHEMA,
            cv.Optional(CONF_ALARM_LEVEL_SOC_LOW): ALARM_LEVEL_SCHEMA,
            cv.Optional(CONF_ALARM_LEVEL_SOH_LOW): ALARM_LEVEL_SCHEMA,
            cv.Optional(CONF_ALARM_LEVEL_MOS_OVERTEMPERATURE): ALARM_LEVEL_SCHEMA,

        }
    )
    .extend(get_cell_voltages_schema())
    .extend(get_temperature_sensors_schema())
    .extend(cv.COMPONENT_SCHEMA)
)


async def setup_conf(config, key, hub):
    if sensor_config := config.get(key):
        sens = await sensor.new_sensor(sensor_config)
        cg.add(getattr(hub, f"set_{key}_sensor")(sens))


async def setup_cell_voltage_conf(config, cell, hub):
    key = get_cell_voltage_key(cell)
    if sensor_config := config.get(key):
        sens = await sensor.new_sensor(sensor_config)
        cg.add(hub.set_cell_voltage_sensor(cell, sens))

async def to_code(config):
    hub = await cg.get_variable(config[CONF_DALY_HKMS_BMS_ID])
    for i in range(1, MAX_CELL_NUMBER + 1):
        await setup_cell_voltage_conf(config, i, hub)
    for i in range(1, MAX_TEMP_NUMBER + 1):
        await setup_conf(config, get_temperature_sensor_key(i), hub)
    for key in TYPES:
        await setup_conf(config, key, hub)
